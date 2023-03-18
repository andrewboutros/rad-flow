import os
import math


def parse_config_file(
    config_filename, booksim_params, radsim_header_params, radsim_knobs
):
    with open(config_filename) as config_file:
        config_file_contents = config_file.readlines()

    for line in config_file_contents:
        line = line.replace("\n", "")
        line = line.replace(" ", "")
        if len(line) != 0:
            split_line = line.split("=")
            param_name = split_line[0]
            if split_line[1][0] == "[":
                param_value = split_line[1][1:-1].split(",")
                if param_value[0].isnumeric():
                    param_value = [int(i) for i in param_value]
            else:
                param_value = split_line[1]
                if param_value.isnumeric():
                    param_value = int(param_value)

            invalid_param = True
            if param_name in booksim_params:
                booksim_params[param_name] = param_value
                invalid_param = False
            if param_name in radsim_header_params:
                radsim_header_params[param_name] = param_value
                invalid_param = False
            if param_name in radsim_knobs:
                radsim_knobs[param_name] = param_value
                invalid_param = False

            if invalid_param:
                print("Config Error: Parameter " + param_name + " is invalid!")
                exit(0)


def print_config(booksim_params, radsim_header_params, radsim_knobs):
    print("*****************************")
    print("**  RAD-FLOW CONFIGURATION **")
    print("*****************************")
    for param in booksim_params:
        print(param + " : " + str(booksim_params[param]))
    for param in radsim_header_params:
        print(param + " : " + str(radsim_header_params[param]))
    for param in radsim_knobs:
        print(param + " : " + str(radsim_knobs[param]))


def generate_booksim_config_files(booksim_params, radsim_header_params, radsim_knobs):
    for i in range(booksim_params["num_nocs"]):
        booksim_config_file = open(
            booksim_params["radsim_root_dir"] + "/sim/noc/noc" + str(i) + "_config", "w"
        )

        # Booksim topology configuration
        booksim_config_file.write("// Topology\n")
        noc_topology = booksim_params["noc_topology"][i]
        if noc_topology == "mesh":
            # A 3D RAD instance is modeled as a concenterated mesh NoC
            if booksim_params["rad_instance"] == "2d":
                booksim_config_file.write("topology = mesh;\n")
            else:
                booksim_config_file.write("topology = cmesh;\n")

            # Booksim does not support assymetric meshes so it is simplified as a square mesh assuming that a simple dim
            # order routing will never use the links/routers outside the specified grid
            noc_dim = booksim_params["noc_dim"][i].split("x")
            larger_noc_dim = int(noc_dim[1])
            if int(noc_dim[0]) > int(noc_dim[1]):
                larger_noc_dim = int(noc_dim[0])
            booksim_config_file.write("k = " + str(larger_noc_dim) + ";\n")
            booksim_config_file.write("n = 2;\n")

            # Booksim supports concentrated meshes of 4 nodes per router only -- RAD-Sim works around that by modeling 3D
            # RAD instances as a concentrated mesh of FPGA node, base die node, and two "empty" nodes by adjusting their IDs
            if booksim_params["rad_instance"] == "3d":
                radsim_header_params["noc_num_nodes"][i] = (
                    larger_noc_dim * larger_noc_dim * 4
                )
                radsim_knobs["noc_num_nodes"][i] = larger_noc_dim * larger_noc_dim * 4
                booksim_config_file.write("c = 4;\n")
                booksim_config_file.write("xr = 2;\n")
                booksim_config_file.write("yr = 2;\n")
            else:
                radsim_header_params["noc_num_nodes"][i] = (
                    larger_noc_dim * larger_noc_dim
                )
                radsim_knobs["noc_num_nodes"][i] = larger_noc_dim * larger_noc_dim

        elif noc_topology == "anynet":
            booksim_config_file.write("topology = anynet;\n")
            booksim_config_file.write(
                "network_file = " + booksim_params["noc_anynet_file"][i] + ";\n"
            )
            if radsim_header_params["noc_num_nodes"][i] == 0:
                print(
                    "Config Error: Number of nodes parameter missing for anynet NoC topologies!"
                )
                exit(0)

        else:
            print("Config Error: This NoC topology is still not supported by RAD-Sim!")
            exit(0)
        booksim_config_file.write("\n")

        # Booksim routing function configuration
        booksim_config_file.write("// Routing\n")
        booksim_config_file.write(
            "routing_function = " + booksim_params["noc_routing_func"][i] + ";\n"
        )
        booksim_config_file.write("\n")

        # Booksim flow control configuration
        booksim_config_file.write("// Flow control\n")
        noc_vcs = booksim_params["noc_vcs"][i]
        noc_num_packet_types = booksim_params["noc_num_packet_types"][i]
        if noc_vcs % noc_num_packet_types != 0:
            print(
                "Config Error: Number of virtual channels has to be a multiple of the number of packet types!"
            )
            exit(0)
        if noc_num_packet_types > 5:
            print("Config Error: RAD-Sim supports up to 5 packet types")
            exit(0)
        noc_num_vcs_per_packet_type = int(noc_vcs / noc_num_packet_types)
        booksim_config_file.write("num_vcs = " + str(noc_vcs) + ";\n")
        booksim_config_file.write(
            "vc_buf_size = " + str(booksim_params["noc_vc_buffer_size"][i]) + ";\n"
        )
        booksim_config_file.write(
            "output_buffer_size = "
            + str(booksim_params["noc_output_buffer_size"][i])
            + ";\n"
        )
        booksim_flit_types = [
            "read_request",
            "write_request",
            "write_data",
            "read_reply",
            "write_reply",
        ]
        vc_count = 0
        for t in range(noc_num_packet_types):
            booksim_config_file.write(
                booksim_flit_types[t] + "_begin_vc = " + str(vc_count) + ";\n"
            )
            vc_count = vc_count + noc_num_vcs_per_packet_type
            booksim_config_file.write(
                booksim_flit_types[t] + "_end_vc = " + str(vc_count - 1) + ";\n"
            )
        booksim_config_file.write("\n")

        # Booksim router architecture and delays configuration
        booksim_config_file.write("// Router architecture & delays\n")
        booksim_config_file.write(
            "router = " + booksim_params["noc_router_uarch"][i] + ";\n"
        )
        booksim_config_file.write(
            "vc_allocator = " + booksim_params["noc_vc_allocator"][i] + ";\n"
        )
        booksim_config_file.write(
            "sw_allocator = " + booksim_params["noc_sw_allocator"][i] + ";\n"
        )
        booksim_config_file.write("alloc_iters = 1;\n")
        booksim_config_file.write("wait_for_tail_credit = 0;\n")
        booksim_config_file.write(
            "credit_delay = " + str(booksim_params["noc_credit_delay"][i]) + ";\n"
        )
        booksim_config_file.write(
            "routing_delay = " + str(booksim_params["noc_routing_delay"][i]) + ";\n"
        )
        booksim_config_file.write(
            "vc_alloc_delay = " + str(booksim_params["noc_vc_alloc_delay"][i]) + ";\n"
        )
        booksim_config_file.write(
            "sw_alloc_delay = " + str(booksim_params["noc_sw_alloc_delay"][i]) + ";\n"
        )
        booksim_config_file.close()


def generate_radsim_params_header(radsim_header_params):
    radsim_params_header_file = open(
        radsim_header_params["radsim_root_dir"] + "/sim/radsim_defines.hpp", "w"
    )
    radsim_params_header_file.write("#pragma once\n\n")
    radsim_params_header_file.write(
        '#define RADSIM_ROOT_DIR "' + radsim_header_params["radsim_root_dir"] + '"\n\n'
    )

    radsim_params_header_file.write("// NoC-related Parameters\n")
    # Finding maximum NoC payload width and setting its definition
    max_noc_payload_width = 0
    for w in radsim_header_params["noc_payload_width"]:
        if w > max_noc_payload_width:
            max_noc_payload_width = w
    radsim_params_header_file.write(
        "#define NOC_LINKS_PAYLOAD_WIDTH " + str(max_noc_payload_width) + "\n"
    )
    # Finding maximum NoC VC count and setting definition for VC ID bitwidth
    max_noc_vcs = 0
    for v in radsim_header_params["noc_vcs"]:
        if v > max_noc_vcs:
            max_noc_vcs = v
    max_vc_id_bitwidth = int(math.ceil(math.log(max_noc_vcs, 2)))
    radsim_params_header_file.write(
        "#define NOC_LINKS_VCID_WIDTH " + str(max_vc_id_bitwidth) + "\n"
    )
    # Setting definition for packet ID bitwidth as directly specified by the user
    packet_id_bitwidth = radsim_header_params["noc_packet_id_width"]
    radsim_params_header_file.write(
        "#define NOC_LINKS_PACKETID_WIDTH " + str(packet_id_bitwidth) + "\n"
    )
    # Finding maximum number of packet types and setting definition for type ID bitwidth & generic packet type mappings
    # to Booksim flit types
    max_num_types = 0
    for t in radsim_header_params["noc_num_packet_types"]:
        if t > max_num_types:
            max_num_types = t
    max_type_id_bitwidth = int(math.ceil(math.log(max_num_types, 2)))
    radsim_params_header_file.write(
        "#define NOC_LINKS_TYPEID_WIDTH " + str(max_type_id_bitwidth) + "\n"
    )
    booksim_flit_types = [
        "READ_REQUEST",
        "WRITE_REQUEST",
        "WRITE_DATA",
        "READ_REPLY",
        "WRITE_REPLY",
    ]
    for i in range(max_num_types):
        radsim_params_header_file.write(
            "#define PACKET_TYPE_" + str(i) + " Flit::" + booksim_flit_types[i] + "\n"
        )
    # Finding maximum NoC node count and setting definition for destination bitwidth
    max_num_nodes = 0
    for n in radsim_header_params["noc_num_nodes"]:
        if n > max_num_nodes:
            max_num_nodes = n
    max_destination_bitwidth = int(math.ceil(math.log(max_num_nodes, 2)))
    radsim_params_header_file.write(
        "#define NOC_LINKS_DEST_WIDTH " + str(max_destination_bitwidth) + "\n"
    )
    # Setting definition for destination interface ID bitwidth
    dest_interface_bitwidth = int(
        math.ceil(math.log(radsim_header_params["max_num_router_dest_interfaces"], 2))
    )
    radsim_params_header_file.write(
        "#define NOC_LINKS_DEST_INTERFACE_WIDTH " + str(dest_interface_bitwidth) + "\n"
    )
    radsim_params_header_file.write("\n")

    radsim_params_header_file.write("// AXI-Streaming Parameters\n")
    # Setting definitions for AXI-Streaming bitwidths
    radsim_params_header_file.write(
        "#define AXIS_MAX_DATAW "
        + str(radsim_header_params["max_axis_tdata_width"])
        + "\n"
    )
    radsim_params_header_file.write(
        "#define AXIS_STRBW " + str(radsim_header_params["axis_tstrb_width"]) + "\n"
    )
    radsim_params_header_file.write(
        "#define AXIS_KEEPW " + str(radsim_header_params["axis_tkeep_width"]) + "\n"
    )
    radsim_params_header_file.write(
        "#define AXIS_USERW " + str(radsim_header_params["axis_tuser_width"]) + "\n"
    )
    radsim_params_header_file.write("\n")

    radsim_params_header_file.write("// Deduced Parameters\n")
    # Setting AXI-Streaming deduced parameter definitions
    radsim_params_header_file.write(
        "#define NOC_LINKS_WIDTH (NOC_LINKS_PAYLOAD_WIDTH + NOC_LINKS_VCID_WIDTH + NOC_LINKS_PACKETID_WIDTH + NOC_LINKS_DEST_WIDTH + NOC_LINKS_DEST_INTERFACE_WIDTH)\n"
    )
    radsim_params_header_file.write("#define AXIS_IDW NOC_LINKS_DEST_INTERFACE_WIDTH\n")
    radsim_params_header_file.write("#define AXIS_DESTW NOC_LINKS_DEST_WIDTH\n")
    radsim_params_header_file.write(
        "#define AXIS_TRANSACTION_WIDTH (AXIS_MAX_DATAW + AXIS_STRBW + AXIS_KEEPW + AXIS_IDW + AXIS_DESTW + AXIS_USERW + 1)\n"
    )
    radsim_params_header_file.write(
        "#define AXIS_TRANSACTION_PAYLOAD_WIDTH (AXIS_IDW + AXIS_MAX_DATAW + AXIS_USERW + 1)\n"
    )
    radsim_params_header_file.write("#define AXIS_TID(t) t.range(AXIS_IDW - 1, 0)\n")
    radsim_params_header_file.write(
        "#define AXIS_TLAST(t) t.range(AXIS_IDW, AXIS_IDW)\n"
    )
    radsim_params_header_file.write(
        "#define AXIS_TUSER(t) t.range(AXIS_USERW + AXIS_IDW, AXIS_IDW + 1)\n"
    )
    radsim_params_header_file.write(
        "#define AXIS_TDATA(t) t.range(AXIS_MAX_DATAW + AXIS_USERW + AXIS_IDW, AXIS_USERW + AXIS_IDW + 1)\n"
    )
    radsim_params_header_file.write(
        "#define AXIS_TDEST(t) t.range(AXIS_DESTW + AXIS_MAX_DATAW + AXIS_USERW + AXIS_IDW, AXIS_MAX_DATAW + AXIS_USERW + AXIS_IDW + 1)\n"
    )
    radsim_params_header_file.write(
        "#define AXIS_PAYLOAD(t) t.range(AXIS_IDW + AXIS_MAX_DATAW + AXIS_USERW, 0)\n\n"
    )

    radsim_params_header_file.write("// AXI-MM Parameters\n")
    # Setting definitions for AXI-MemoryMapped bitwidths
    radsim_params_header_file.write(
        "#define AXI_IDW " + str(radsim_header_params["axi_id_width"]) + "\n"
    )
    radsim_params_header_file.write(
        "#define AXI_USERW " + str(radsim_header_params["axi_user_width"]) + "\n"
    )
    radsim_params_header_file.write(
        "#define AXI_MAX_DATAW "
        + str(radsim_header_params["max_axi_data_width"])
        + "\n\n"
    )

    radsim_params_header_file.write("// AXI-MM Constants\n")
    # Setting AXI-MemoryMapped deduced parameter definitions
    radsim_params_header_file.write("#define AXI_ADDRW 64\n")
    radsim_params_header_file.write("#define AXI_LENW 8\n")
    radsim_params_header_file.write("#define AXI_SIZEW 3\n")
    radsim_params_header_file.write("#define AXI_BURSTW 2\n")
    radsim_params_header_file.write("#define AXI_RESPW 2\n")
    radsim_params_header_file.write(
        "#define AXI_CTRLW (AXI_LENW + AXI_SIZEW + AXI_BURSTW)\n"
    )
    radsim_params_header_file.write(
        "#define AXI_TRANSACTION_MAX_WIDTH (AXI_MAX_DATAW + AXI_RESPW + 1 + AXI_IDW + AXI_USERW)\n"
    )
    radsim_params_header_file.write("#define AXI_USER(t) t.range(AXI_USERW - 1, 0)\n")
    radsim_params_header_file.write(
        "#define AXI_CTRL(t) t.range(AXI_USERW + AXI_CTRLW - 1, AXI_USERW)\n"
    )
    radsim_params_header_file.write(
        "#define AXI_ADDR(t) t.range(AXI_USERW + AXI_CTRLW + AXI_ADDRW - 1, AXI_USERW + AXI_CTRLW)\n"
    )
    radsim_params_header_file.write(
        "#define AXI_RESP(t) t.range(AXI_USERW + AXI_RESPW - 1, AXI_USERW)\n"
    )
    radsim_params_header_file.write(
        "#define AXI_LAST(t) t.range(AXI_USERW + AXI_RESPW, AXI_USERW + AXI_RESPW)\n"
    )
    radsim_params_header_file.write(
        "#define AXI_DATA(t) t.range(AXI_USERW + AXI_RESPW + AXI_MAX_DATAW, AXI_USERW + AXI_RESPW + 1)\n\n"
    )

    radsim_params_header_file.write("#define AXI_TYPE_AR 0\n")
    radsim_params_header_file.write("#define AXI_TYPE_AW 1\n")
    radsim_params_header_file.write("#define AXI_TYPE_W 2\n")
    radsim_params_header_file.write("#define AXI_TYPE_R 3\n")
    radsim_params_header_file.write("#define AXI_TYPE_B 4\n")
    radsim_params_header_file.write("#define AXI_NUM_RSP_TYPES 2\n")
    radsim_params_header_file.write("#define AXI_NUM_REQ_TYPES 3\n\n")

    radsim_params_header_file.close()


def generate_radsim_config_file(radsim_knobs):
    radsim_config_file = open(
        radsim_header_params["radsim_root_dir"] + "/sim/radsim_knobs", "w"
    )
    for param in radsim_knobs:
        radsim_config_file.write(param + " ")
        if isinstance(radsim_knobs[param], list):
            for value in radsim_knobs[param]:
                radsim_config_file.write(str(value) + " ")
            radsim_config_file.write("\n")
        else:
            radsim_config_file.write(str(radsim_knobs[param]) + "\n")
    radsim_config_file.close()


config_filename = "../rad-flow.config"

booksim_params = {
    "radsim_root_dir": os.getcwd() + "/rad-sim",
    "rad_instance": "2d",
    "num_nocs": 1,
    "noc_topology": ["mesh"],
    "noc_anynet_file": [os.getcwd() + "/rad-sim/sim/noc/anynet_file"],
    "noc_dim": ["8x8"],
    "noc_routing_func": ["dim_order"],
    "noc_vcs": [3],
    "noc_vc_buffer_size": [4],
    "noc_output_buffer_size": [4],
    "noc_num_packet_types": [3],
    "noc_router_uarch": ["iq"],
    "noc_vc_allocator": ["islip"],
    "noc_sw_allocator": ["islip"],
    "noc_credit_delay": [2],
    "noc_routing_delay": [0],
    "noc_vc_alloc_delay": [1],
    "noc_sw_alloc_delay": [1],
}
radsim_header_params = {
    "radsim_root_dir": os.getcwd() + "/rad-sim",
    "noc_payload_width": [166],
    "noc_packet_id_width": 32,
    "noc_vcs": [3],
    "noc_num_packet_types": [3],
    "noc_num_nodes": [0],
    "max_num_router_dest_interfaces": 32,
    "max_axis_tdata_width": 640,
    "axis_tkeep_width": 8,
    "axis_tstrb_width": 8,
    "axis_tuser_width": 32,
    "axi_id_width": 8,
    "axi_user_width": 64,
    "max_axi_data_width": 512,
}
radsim_knobs = {
    "radsim_root_dir": os.getcwd() + "/rad-sim",
    "radsim_user_design_root_dir": os.getcwd() + "/rad-sim/example-designs/npu",
    "num_nocs": 1,
    "noc_period": [1.0],
    "noc_vcs": [3],
    "noc_payload_width": [166],
    "noc_num_nodes": [0],
    "noc_placement_file": ["noc.place"],
    "adapter_period": [1.25],
    "adapter_fifo_size": [16],
    "adapter_obuff_size": [2],
    "adapter_in_arbiter": ["fixed_rr"],
    "adapter_out_arbiter": ["priority_rr"],
    "adapter_vc_mapping": ["direct"],
    "module_period": [5.0],
    "sim_driver_period": 5.0,
    "log_verbosity": 0,
    "num_traces": 2,
    "num_trace_modules": 1,
    "trace_names": ["trace0", "trace1"],
    "dram_controllers": 1,
    "dram_controller_period": [2.0],
    "dram_controller_queue_size": [64],
    "dram_config_file": ["HBM2_8Gb_x128"],
}

parse_config_file(config_filename, booksim_params, radsim_header_params, radsim_knobs)
# print_config(booksim_params, radsim_header_params, radsim_knobs)
generate_booksim_config_files(booksim_params, radsim_header_params, radsim_knobs)
generate_radsim_params_header(radsim_header_params)
generate_radsim_config_file(radsim_knobs)
print("Finished RAD-Flow Configuration!")
