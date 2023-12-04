import os, sys
from bitstring import BitArray
from typing import List, Dict, Any
from dataclasses import dataclass
from collections import OrderedDict

import operator

import argparse
from collections import defaultdict


embeddings_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "embedding_tables")
mem_init_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "ext_mem_init")
os.makedirs(mem_init_dir, exist_ok=True)

# Traffic gen specific files
def compile_traffic_gen(input_insts: List[Dict[str, Any]]) -> None:
    """
        generate traffic gen mem req instructions
    """

# not sure about this (but should be if 1024 bits per line)
# address_width = 32





def addr_format(addr: int, addr_width: int) -> str:
    return format(addr, f"0{addr_width}x")


# Conversions
def signed_int_2_bin(data: int, bitwidth: int) -> str:
    if data < 0:
        signed_bin = bin(data & (2**bitwidth)-1)[2:].zfill(bitwidth)
    else:
        signed_bin = bin(data)[2:].zfill(bitwidth)
    
    return signed_bin  

def bin_2_signed_int(data: str) -> int:
    return BitArray(bin=data).int

def data_int_to_mem_str(data: int, addr_line_sz: int) -> str:
    """
        Convert a single int to a string of bits which can be written to a single address line
    """
    return bin(data)[2:].zfill(addr_line_sz)

def data_str_to_ints(data_str: str, bitwidth: int) -> List[int]:
    return [BitArray(bin=data_str[i:i + bitwidth]).int for i in range(0, len(data_str), bitwidth)]

def data_eles_to_str(data_eles: List[int], bitwidth: int) -> str:
    return "".join([signed_int_2_bin(data, bitwidth) for data in data_eles])

def decode_mem_contents(channel: int, bitwidth: int, mem_path: str = None, addr_width: int = 32) -> Dict[str, List[int]]:
    print(f" CHANNEL {channel} ")
    if mem_path is None:
        with open (os.path.join(embeddings_dir, f"channel_{channel}.dat"), "r") as f:
            text = f.read()
    else:
        with open(mem_path, "r") as f:
            text = f.read()
    ch_out = {}
    for line in text.split("\n"):
        if len(line.split(" ")) == 2: 
            addr, data = line.split(" ")
            elements = [data[i:i + bitwidth] for i in range(0, len(data), bitwidth)]
            # Assume addresses are in hex (no 0x prefix)
            print(f"[ {addr_format(int(addr,16), addr_width)} ]: {' '.join([str(BitArray(bin=e).int) for e in elements])}" )
            ch_out[addr_format(int(addr,16), addr_width)] = [BitArray(bin=e).int for e in elements]
            # ch_out.append({addr: [BitArray(bin=e).int for e in elements]})
    return ch_out
        




# def bin_2_signed_int(data: str) -> int:
#     if data[0] == "1":
#         return -int(data[1:], 2)
#     else:
#         return int(data, 2)

def write_mem_contents(channel: int, mem_contents: Dict[str, int], bitwidth: int, addr_line_sz: int, addr_width: int = 32):
    # print(f" CHANNEL {channel} ")
    # clear pre existing file
    open(os.path.join(mem_init_dir, f"channel_{channel}.dat"), "w").close()    
    for addr, data_eles in mem_contents.items():
        # all for a single address
        assert len(data_eles) * bitwidth <= addr_line_sz, f"Error in write_mem_contents: data elements must fit in a single address line"
        # get data elements to binary
        data_strs = [signed_int_2_bin(data, bitwidth) for data in data_eles]
        addr_data = "".join(data_strs).zfill(addr_line_sz)
        with open (os.path.join(mem_init_dir, f"channel_{channel}.dat"), "a+") as f:
            print(f"{addr_format(int(addr), addr_width)} {addr_data}", file=f)



@dataclass 
class AXIPort:
    name: str # name of port corresponding to the hbm_traffic.place file
    type: str # AXI-S or AXI-MM
    is_master: bool # True if master, False if slave
    noc_loc: int # index of the noc port corresponding to the hbm_traffic.place file
    noc_idx: int # index of the entire NoC which this port/router exists in (if only using a single noc just use 0)
    id: int = None # The id related to number of total insts of this module (or inst multiple subtypes of single module)

    supported_types = ["aximm", "axis"]
    def __post_init__(self):
        if (self.type not in self.supported_types):
            raise Exception(f"Error in AXIPort: {self.type} is not a supported type")

    def get_port_info_line(self) -> str:        
        return f"{self.name} {self.noc_idx} {self.noc_loc} {self.type} {int(self.is_master)}" 
    
    def get_port_placement_info_line(self) -> str:
        return f"{self.name} {self.noc_idx} {self.noc_loc} {self.type}"

    # def get_port_id_str(self) -> str:
    #     return f"{self.name}"

@dataclass
class MemReqInstruction:
    inst_address: int # instructions at the same address will be sent on the same cycle
    mem_req_module_id: int
    src_port: str
    dst_port: str 
    target_channel: int 
    target_address: int 
    write_data: int
    write_en: bool


@dataclass
class HWModule:
    name: str
    inst_name: str
    ports: List[AXIPort]
    id: int = None # The id related to number of total insts of this module (or inst multiple subtypes of single module)
    
@dataclass
class ExtMem:
    type: str # ddr, hbm, etc
    num_channels: int
    addr_width: int # using 32 for now not sure TODO figure out for DDR & HBM
    data_width: int # how much data fits into a single address, using 1024 for now not sure TODO figure out for DDR & HBM
    ch_id_range: List[int] = None # range of channel ids for this external memory
    id: int = None # id for number of external memories of this type
    # For verification
    mem_contents: List[Dict[str, List[int]]] = None # [ {addr: [e for ele in data]} for ch in range(num_channels)] 
    inst_mem_req_contents: Dict[str, Any] = None # { inst_address: {addr: [e for ele in data]} for ch in range(num_channels) }
    inst_mem_writes: Dict[str, Any] = None # { inst_address: {addr: [e for ele in data]} for ch in range(num_channels) }

    # POST INIT FIELDS
    # Mem controllers exist here
    mem_ctrl_modules: List[HWModule] = None # (starts uninitialized) one for each channel (for now)

    def __post_init__(self):
        self.mem_contents = [{} for _ in range(self.num_channels)]
        self.inst_mem_req_contents = {}
        self.inst_mem_writes = {}

    def init_mem_ctrl_modules(self, noc_locs: List[int]):
        self.mem_ctrl_modules = [
            HWModule(
                name=f"ext_mem_ctrl",
                inst_name=f"{self.type}_mem_ctrl",
                ports=[
                    AXIPort(f"{self.type}_mem_ctrl_{ch}.aximm_interface", "aximm", False, noc_locs[ch], 0) 
                    for ch in range(self.num_channels)
                ]
            ) 
        ]

    def get_mem_content_lines(self) -> List[str]:
        """
            Dump the contents of the memory to a file
        """
        ret_lines = []
        for ch in range(self.num_channels):
            for addr, data in self.mem_contents[ch].items():
                ret_lines.append(f"[{ch}] [{addr}] {' '.join([str(e) for e in data])}")
        return ret_lines



@dataclass
class RADSimModules:
    hw_modules: List[HWModule] # Design HW modules (including ext mem modules)
    ext_mems: List[ExtMem]
    # Count of instantiations of each type of HW module, this is used for port / module inst naming later
    # module_cnts: Dict[str, int] 

    def __post_init__(self):
        # get counts for each module type (including ext mems)
        module_cnts = { module.name: 0 for module in self.hw_modules}
        for mod_1 in self.hw_modules:
            for mod_2 in module_cnts.keys():
                if mod_1.name == mod_2:
                    mod_1.id = module_cnts[mod_2] # set module id to be the number of modules of this type
                    module_cnts[mod_2] += 1
                    break 

        ext_mem_ctrl_cnts = { f"{ext_mem.type}_{mem_ctrl_mod.name}": 0 for ext_mem in self.ext_mems for mem_ctrl_mod in ext_mem.mem_ctrl_modules}
        for ext_mem in self.ext_mems:
        # for mem_type in set([ext_mem.type for ext_mem in self.ext_mems]):
            for mem_key in ext_mem_ctrl_cnts.keys():
                for i, mod_1 in enumerate(ext_mem.mem_ctrl_modules):
                    if mem_key == f"{ext_mem.type}_{mod_1.name}":
                        mod_1.id = ext_mem_ctrl_cnts[mem_key]
                        ext_mem.id = ext_mem_ctrl_cnts[mem_key]
                        ext_mem_ctrl_cnts[mem_key] += 1
                        break
        

    def rename_modules(self):
        """
            Rename modules to be unique
        """
        # renaming hw_modules
        for module in self.hw_modules:
            module.inst_name = f"{module.name}_{module.id}_inst"
            mod_mem_str = "" #"mem_channel" if "mem_ctrl" in module.name else "interface"
            for id, port in enumerate(module.ports):
                mas_slv_str = "master" if port.is_master else "slave"
                port.name = f"{module.inst_name}.{port.type}_{mas_slv_str}_{mod_mem_str}_{id}"
        # renaming ext mem modules
        for ext_mem in self.ext_mems:
            for module in ext_mem.mem_ctrl_modules:
                module.inst_name = f"{ext_mem.type}_{module.name}_{module.id}_inst"
                for id, port in enumerate(module.ports):
                    mas_slv_str = "master" if port.is_master else "slave"
                    port.name = f"{module.inst_name}.{port.type}_{mas_slv_str}_mem_channel_{id}"

    def write_placement_file(self, outfile: str) -> None:
        """
            Write the placement file for the traffic gen
        """
        all_ports = [port for module in self.hw_modules for port in module.ports] + [port for ext_mem in self.ext_mems for module in ext_mem.mem_ctrl_modules for port in module.ports]
        with open (outfile, "w") as file:
            for port in all_ports:
                print(port.get_port_placement_info_line(), file=file)


    def write_module_insts_config(self, outfile: str) -> None:
        """
            Write the module instantiation file for the traffic gen
        """
        # Written out with information associated with a particular module bounded by "module" "endmodule"

        with open(outfile, "w") as f:
            for module in self.hw_modules + [module for ext_mem in self.ext_mems for module in ext_mem.mem_ctrl_modules]:
                print(f"module {module.name} {module.inst_name}", file=f)
                # print(f"{[port.get_port_info_line() for port in module.ports]}", file=f)
                for port in module.ports:
                    print(port.get_port_info_line(), file=f)
                print("endmodule", file=f)

    
def write_mem_req_instructions(mem_req_instructions: List[MemReqInstruction], addr_width: int, mem_data_width: int, outfile: str) -> None:
    with open(outfile, "w") as f:
        # print the number of instruction addresses as header
        print(f"{len(set([inst.inst_address for inst in mem_req_instructions]))}", file=f)
        for inst in mem_req_instructions:
            print(f"{addr_format(inst.inst_address, 4)} {inst.mem_req_module_id} {inst.src_port} {inst.dst_port} {inst.target_channel} {addr_format(inst.target_address, addr_width)} {signed_int_2_bin(inst.write_data, mem_data_width)} {int(inst.write_en)}", file=f)



# @dataclass 
# class MemContents:
#     """
#         Models the contents of a single external memory
#     """
#     addr_width: int
#     data_width: int
#     contents: Dict[str, List[int]]


@dataclass
class MemModel:
    """
        Models execution of read / writes to / from single external memory
    """
    rad_sim_modules: RADSimModules
    insts: List[MemReqInstruction]
    ch_mapping: Dict[int, int]

    ele_bitwidth: int = 16

    def __post_init__(self):
        # Sort instructions by their inst address (order of execution)
        self.insts = sorted(self.insts, key=operator.attrgetter('inst_address'))

    def sim_execution(self, sim_outdir: str):
        """
            Simulate the execution of the instructions, writes a golden reference of what content should be in which memory addresses
        """
        # Storing all read mem instructions
        for inst_addr in set([inst.inst_address for inst in self.insts if not inst.write_en]):
            for mem in self.rad_sim_modules.ext_mems:
                mem.inst_mem_req_contents[addr_format(inst_addr,4)] = [{} for _ in range(mem.num_channels)]
        for inst_addr in set([inst.inst_address for inst in self.insts if inst.write_en]):
            for mem in self.rad_sim_modules.ext_mems:
                mem.inst_mem_writes[addr_format(inst_addr,4)] = [{} for _ in range(mem.num_channels)]

        # Assuming non contention for same memory addresses (TODO figure this out later)
        for inst in self.insts:
            # Find memory target for this instruction
            for mem in self.rad_sim_modules.ext_mems:
                # Determine which memory this instruction is relevant to
                if inst.target_channel in mem.ch_id_range:
                    # Write data to memory
                    if inst.write_en:
                        mem.mem_contents[self.ch_mapping[inst.target_channel]][addr_format(inst.target_address, mem.addr_width)] = data_str_to_ints(data_int_to_mem_str(inst.write_data, mem.data_width), self.ele_bitwidth)
                        mem.inst_mem_writes[addr_format(inst.inst_address, 4)][inst.target_channel] = {addr_format(inst.target_address, mem.addr_width): mem.mem_contents[inst.target_channel][addr_format(inst.target_address, mem.addr_width)]}
                    # Read data from memory
                    else:
                        mem.inst_mem_req_contents[addr_format(inst.inst_address, 4)][inst.target_channel] = {addr_format(inst.target_address, mem.addr_width): mem.mem_contents[inst.target_channel][addr_format(inst.target_address, mem.addr_width)]}
                    break
        # After all reads and writes dump the inst_mem_req_contents to a golden reference file
        for mem in self.rad_sim_modules.ext_mems:
            with open(os.path.join(sim_outdir, f"{mem.mem_ctrl_modules[0].inst_name}_golden_mem.out"), "w") as f:
                print()
                print(f"Writing golden reference for {mem.mem_ctrl_modules[0].inst_name}")
                if all( [len(contents) == 0 for key, inst_addr_contents in mem.inst_mem_req_contents.items() for contents in inst_addr_contents]):
                    print("0", file=f)
                    print("No reads to this memory")
                else:
                    # print number of outputs
                    print(len(mem.inst_mem_req_contents), file=f) 
                    for inst_addr, mem_req_contents in mem.inst_mem_req_contents.items():
                        for ch_id, ch_contents in enumerate(mem_req_contents):
                            for mem_addr, mem_data in ch_contents.items():
                                # ' '.join([str(e) for e in mem_data])
                                print( f"[{inst_addr}] [{ch_id}] [{mem_addr}] {' '.join([str(e) for e in mem_data])}")
                                mem_line = f"{inst_addr} {ch_id} {mem_addr} {data_eles_to_str(mem_data, self.ele_bitwidth)}"
                                print(mem_line, file=f)
            with open(os.path.join(sim_outdir, f"{mem.mem_ctrl_modules[0].inst_name}_golden_wr.out"), "w") as f:
                print()
                print(f"Writing golden reference for write transactions for {mem.mem_ctrl_modules[0].inst_name}")
                if all( [len(contents) == 0 for key, inst_addr_contents in mem.inst_mem_req_contents.items() for contents in inst_addr_contents]):
                    print("0", file=f)
                    print("No Writes to this memory")
                else:
                    # print number of outputs
                    print(len(mem.inst_mem_writes), file=f) 
                    for inst_addr, mem_req_contents in mem.inst_mem_writes.items():
                        for ch_id, ch_contents in enumerate(mem_req_contents):
                            for mem_addr, mem_data in ch_contents.items():
                                # ' '.join([str(e) for e in mem_data])
                                print( f"[{inst_addr}] [{ch_id}] [{mem_addr}] {' '.join([str(e) for e in mem_data])}")
                                mem_line = f"{inst_addr} {ch_id} {mem_addr} {data_eles_to_str(mem_data, self.ele_bitwidth)}"
                                print(mem_line, file=f)
                            # print(mem_line)
        



def main():

    # Parse args
    args = argparse.ArgumentParser()

    num_mem_channels = 18
    bitwidth = 16
    # addr_list = [0, 64000, 128000]
    # data_list = [1, 2, -3]
    mem_contents = {
        "0": [1, 2, -3],
        "64000": [-4, -5, -6],
        "128000": [7, -8, 9]
    }
    
    # Create traffic gen layout
    num_hbms = 2
    num_ddrs = 2
    hbm_channels = 8
    ddr_channels = 1
    addr_line_sz = 1024
    addr_width = 32

    # Init Placement


    # Init Ext Mem
    ext_mems = [
        # TODO fix the placement to be less dumb
        *[ ExtMem("ddr", ddr_channels, addr_width, addr_line_sz, ch_id_range=[i]) for i in range(num_ddrs)],
        *[ ExtMem("hbm", hbm_channels, addr_width, addr_line_sz, ch_id_range=[j + i * hbm_channels + num_ddrs for j in range(hbm_channels)]) for i in range(num_hbms)],
    ]
    ddr_noc_locs = [[1], [71]]
    hbm_noc_locs = [
        [i for i in range(2, 9 + 1) ],
        [i for i in range(72, 79 + 1) ],
    ]
    for mem in ext_mems:
        if mem.type == "ddr":
            mem.init_mem_ctrl_modules(ddr_noc_locs.pop(0))
        else:
            mem.init_mem_ctrl_modules(hbm_noc_locs.pop(0))

    # Init Traffic Gen
    mem_req_modules = [
        HWModule(
            name="black_box",
            inst_name=f"black_box_0_inst",
            ports=[
                AXIPort("black_box_0_inst.aximm_interface", "aximm", True, 21, 0),
                AXIPort("black_box_0_inst.axis_interface", "axis", True, 81, 0),
            ]
        )
    ]



    # Add collector module
    collector = HWModule(
        name="output_collector",
        inst_name=f"output_collector_inst",
        ports = [AXIPort("output_collector_inst.axis_interface", "axis", False, 31, 0)]
    )

    # Init RADSimModules Object for port / module naming, this is needed when we have multiple insts of same module
    all_modules = mem_req_modules + [collector]

    rad_sim_modules = RADSimModules(
        hw_modules = all_modules,
        ext_mems = ext_mems,
    )
    rad_sim_modules.rename_modules()

    # Write placement file
    placement_outfile = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "hbm_traffic.place")
    rad_sim_modules.write_placement_file(placement_outfile)

    num_wr_rds = 4
    # For this test we will have a bunch of instructions which do X writes and then Y reads -> after this we do % writes and then % reads (for N transactions)
    mem_req_instructions = [
        # incrementing the write address by 16 each time, as I'm not sure what the address line size is
        *[MemReqInstruction(0x0 + i, 0, "black_box_0_inst.aximm_master__0" , "ddr_ext_mem_ctrl_0_inst.aximm_slave_mem_channel_0", 0, i << 6, i, True) for i in range(num_wr_rds)], # Writes
        *[MemReqInstruction(0x0 + i + num_wr_rds, 0, "black_box_0_inst.aximm_master__0", "ddr_ext_mem_ctrl_0_inst.aximm_slave_mem_channel_0", 0, i << 6, i, False) for i in range(num_wr_rds)], # Reads
    ]
    

    traffic_gen_outdir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "traffic_gen")
    # Create module instantiation file 
    module_inst_outfile = os.path.join(os.path.dirname(os.path.realpath(__file__)), "traffic_gen", "traffic_gen.cfg")
    rad_sim_modules.write_module_insts_config(module_inst_outfile)

    # Create instructions for all mem_req modules
    inst_outfile = os.path.join(os.path.dirname(os.path.realpath(__file__)), "traffic_gen", "insts.in")
    write_mem_req_instructions(mem_req_instructions, ext_mems[0].addr_width, ext_mems[0].data_width, inst_outfile)

    # Create output golden reference
    golden_outfile = os.path.join(os.path.dirname(os.path.realpath(__file__)), "traffic_gen", "golden.out")
    # Ex golden output 
    # Wr: 1 if the write was sucessful, 0 if not (detailed mem info will be verified by dump)


    # Mapping for total channel index to channel indx for each memory
    ch_mapping = {
        # DDRs
        0: 0,
        1: 0,
        # HBMs
        **{ i + num_ddrs * ddr_channels: i for i in range(hbm_channels) },
        **{ i + num_ddrs * ddr_channels + hbm_channels: i for i in range(hbm_channels)},
    }

    cur_mem_contents = [{} for _ in range(num_mem_channels)]
    ### DOING MEMORY INIT + DECODING ###
    for ch in range(num_mem_channels):
        # decode_mem_contents(ch, bitwidth)    
        write_mem_contents(ch, mem_contents, bitwidth, addr_line_sz)
        # Initialize the mem contents from intialized memory values
        cur_mem_contents[ch] = decode_mem_contents(ch, bitwidth, mem_path=os.path.join(mem_init_dir, f"channel_{ch}.dat"))
        for mem in rad_sim_modules.ext_mems:
            if ch in mem.ch_id_range:
                mem.mem_contents[ch_mapping[ch]] = cur_mem_contents[ch]
                break
    for mem in rad_sim_modules.ext_mems:
        mem_contents_lines = mem.get_mem_content_lines()
        print()
        print(mem.mem_ctrl_modules[0].inst_name)    
        for line in mem_contents_lines:
            print(line)

    ### SIMULATE MEMORY REQUESTS ###
    mem_sim = MemModel(rad_sim_modules, mem_req_instructions, ch_mapping)
    mem_sim.sim_execution(traffic_gen_outdir)
    






if __name__ == "__main__":
    main()


