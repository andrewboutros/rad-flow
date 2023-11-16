import os, sys
from bitstring import BitArray
from typing import List, Dict, Any
from dataclasses import dataclass
from collections import OrderedDict

import argparse



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


def decode_mem_contents(channel: int, bitwidth: int, mem_path: str = None, addr_width: int = 32):
    print(f" CHANNEL {channel} ")
    if mem_path is None:
        with open (os.path.join(embeddings_dir, f"channel_{channel}.dat"), "r") as f:
            text = f.read()
    else:
        with open(mem_path, "r") as f:
            text = f.read()

    for line in text.split("\n"):
        if len(line.split(" ")) == 2: 
            addr, data = line.split(" ")
            elements = [data[i:i + bitwidth] for i in range(0, len(data), bitwidth)]
            # Assume addresses are in hex (no 0x prefix)
            print(f"[ {addr_format(int(addr,16), addr_width)} ]: {' '.join([str(BitArray(bin=e).int) for e in elements])}" )
            # if "0x" in addr:
            #     formatted_addr = f"0x{addr.replace('0x', '').zfill(addr_width)}"
            #     print(f"[ {formatted_addr} ]: {' '.join([str(BitArray(bin=e).int) for e in elements])}" )
            
            # else:
            #     print(f"[ 0x{(int(addr, 16))} ]: {' '.join([str(BitArray(bin=e).int) for e in elements])}" )

def signed_int_2_bin(data: int, bitwidth: int) -> str:
    if data < 0:
        signed_bin = bin(data & (2**bitwidth)-1)[2:].zfill(bitwidth)
    else:
        signed_bin = bin(data)[2:].zfill(bitwidth)
    
    return signed_bin  

def bin_2_signed_int(data: str) -> int:
    return BitArray(bin=data).int


# def bin_2_signed_int(data: str) -> int:
#     if data[0] == "1":
#         return -int(data[1:], 2)
#     else:
#         return int(data, 2)

def write_mem_contents(channel: int, mem_contents: Dict[str, int], bitwidth: int, addr_line_sz: int, addr_width: int = 32):
    print(f" CHANNEL {channel} ")
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

    supported_types = ["aximm", "axis"]
    def __post_init__(self):
        if (self.type not in self.supported_types):
            raise Exception(f"Error in AXIPort: {self.type} is not a supported type")

    def get_port_info_line(self) -> str:        
        return f"{self.name} {self.noc_idx} {self.noc_loc} {self.type} {int(self.is_master)}" 

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
    ports: List[AXIPort]

@dataclass
class ExtMem:
    type: str # ddr, hbm, etc
    num_channels: int
    addr_width: int # using 32 for now not sure TODO figure out for DDR & HBM
    data_width: int # how much data fits into a single address, using 1024 for now not sure TODO figure out for DDR & HBM
    # Mem controllers exist here
    
    # POST INIT FIELDS
    mem_ctrl_modules: List[HWModule] = None # (starts uninitialized) one for each channel (for now)

    def init_mem_ctrl_modules(self, noc_locs: List[int]):
        self.mem_ctrl_modules = [
            HWModule(
                name=f"{self.type}_mem_ctrl",
                ports=[
                    AXIPort(f"{self.type}_mem_ctrl_{ch}.aximm_interface", "aximm", False, noc_locs[ch], 0), # Todo 
                ]
            ) for ch in range(self.num_channels)
        ]


def write_module_insts_config(module_insts: List[HWModule], outfile: str) -> None:
    """
        Write the module instantiation file for the traffic gen
    """
    # Written out with information associated with a particular module bounded by "module" "endmodule"

    with open(outfile, "w") as f:
        for module in module_insts:
            print(f"module {module.name}", file=f)
            # print(f"{[port.get_port_info_line() for port in module.ports]}", file=f)
            for port in module.ports:
                print(port.get_port_info_line(), file=f)
            print("endmodule", file=f)

    
def write_mem_req_instructions(mem_req_instructions: List[MemReqInstruction], addr_width: int, mem_data_width: int, outfile: str) -> None:
    with open(outfile, "w") as f:
        # print the number of instruction addresses as header
        print(len(set([inst.inst_address for inst in mem_req_instructions])), file=f)
        for inst in mem_req_instructions:
            print(f"{addr_format(inst.inst_address, 4)} {inst.mem_req_module_id} {inst.src_port} {inst.dst_port} {inst.target_channel} {addr_format(inst.target_address, addr_width)} {signed_int_2_bin(inst.write_data, mem_data_width)} {int(inst.write_en)}", file=f)



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

    # Init Ext Mem
    ext_mems = [
        # TODO fix the placement to be less dumb
        *[ ExtMem("ddr", ddr_channels, addr_width, addr_line_sz) for i in range(num_ddrs)],
        *[ ExtMem("hbm", hbm_channels, addr_width, addr_line_sz) for i in range(num_hbms)],
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
            ports=[
                AXIPort("black_box_0_inst.aximm_interface", "aximm", True, 21, 0),
                AXIPort("black_box_0_inst.axis_interface", "axis", True, 81, 0),
            ]
        )
    ]

    num_wr_rds = 100
    # For this test we will have a bunch of instructions which do X writes and then Y reads -> after this we do % writes and then % reads (for N transactions)
    mem_req_instructions = [
        # incrementing the write address by 16 each time, as I'm not sure what the address line size is
        *[MemReqInstruction(0x0, 0, "black_box_0_inst.aximm_interface", "ddr_mem_ctrl_0.aximm_interface", 0, i << 4, i, True) for i in range(num_wr_rds)], # Writes
        *[MemReqInstruction(0x0, 0, "black_box_0_inst.aximm_interface", "ddr_mem_ctrl_0.aximm_interface", 0, i << 4, i, False) for i in range(num_wr_rds)], # Reads
    ]

    # Create module instantiation file 
    module_inst_outfile = os.path.join(os.path.dirname(os.path.realpath(__file__)), "traffic_gen", "traffic_gen.cfg")
    write_module_insts_config(mem_req_modules + [module for ext_mem in ext_mems for module in ext_mem.mem_ctrl_modules ], module_inst_outfile)

    # Create instructions for all mem_req modules
    inst_outfile = os.path.join(os.path.dirname(os.path.realpath(__file__)), "traffic_gen", "insts.in")
    write_mem_req_instructions(mem_req_instructions, ext_mems[0].addr_width, ext_mems[0].data_width, inst_outfile)
    ### DOING MEMORY INIT + DECODING ###
    for ch in range(num_mem_channels):
        decode_mem_contents(ch, bitwidth)    
        write_mem_contents(ch, mem_contents, bitwidth, addr_line_sz)
        decode_mem_contents(ch, bitwidth, mem_path=os.path.join(mem_init_dir, f"channel_{ch}.dat"))
        break






if __name__ == "__main__":
    main()


