# Port Mapping Parser Script
# Parses inputs and outputs from top-level Verilog designs and maps it to the port.map format used by Wrapper Generation
# Support for AXI-S, AXI-MM interfaces parsing
# Arguments:
#   [1] => Path to the design folder
#   [2...] => Verilog files for top-level designs
import argparse
import re
import verilog_parser as vlog
from pathlib import Path

verilog_range_regex = "\[(\d*):(\d*)\]"
verilog_axis_regex = "axis_(.*)_(.*)"
verilog_aximm_regex = "aximm_(.*)_(.*)"

axis_slave_input_ports = ["tvalid", "tdata", "tstrb", "tkeep", "tlast", "tid", "tdest", "tuser"]
aximm_slave_input_ports = ["awvalid", "awid", "awaddr", "awlen", "awsize", "awburst", "awuser",
                           "wvalid", "wid", "wdata", "wlast", "wuser",
                           "bready",
                           "arvalid", "arid", "araddr", "arlen", "arsize", "arburst", "aruser",
                           "rready"]

# Determines the width of the port. Outputs the width if successful, "?" if failure
def determine_port_width(port):
    if not port.data_type:
        return 1
    
    port_properties = port.data_type.split()
    # explicitly define test cases, default behavior if no cases match is to output ?
    if len(port_properties) == 1:
        indices = re.match(verilog_range_regex, port_properties[0])
        if port_properties[0] == "reg" or port_properties[0] == "logic":
            return 1
        elif indices and len(indices.groups()) == 2:
            return int(indices.groups()[0]) - int(indices.groups()[1]) + 1
    elif len(port_properties) == 2:
        indices = re.match(verilog_range_regex, port_properties[1])
        if indices and len(indices.groups()) == 2:
            return int(indices.groups()[0]) - int(indices.groups()[1]) + 1
    
    return "?"

# RegEx pattern matching for AXI-S ports. Returns capture groups with information on the AXI-S interface name, and the AXI-S port.
def match_axis_regex(port):
    signal = re.match(verilog_axis_regex, port.name)
    return signal.groups()

# RegEx pattern matching for AXI-MM ports. Returns capture groups with information on the AXI-MM interface name, and the AXI-MM port.
def match_aximm_regex(port):
    signal = re.match(verilog_aximm_regex, port.name)
    return signal.groups()

# Determines if a port is an AXI-S port
def is_axis_port(port):
    if re.match(verilog_axis_regex, port.name):
        return True
    return False

# Determines if a port is an AXI-MM port
def is_aximm_port(port):
    if re.match(verilog_aximm_regex, port.name):
        return True
    return False

# Determines if the role for an AXI-S port has already been saved
def is_axis_role_found(axis_roles, port):
    (axis_interface, axis_port) = match_axis_regex(port)
    if axis_interface in axis_roles:
        return True
    return False

# Determines if the role for an AXI-MM port has already been saved
def is_aximm_role_found(aximm_roles, port):
    (aximm_interface, aximm_port) = match_aximm_regex(port)
    if aximm_interface in aximm_roles:
        return True
    return False

# Determines the AXI-S roles for a given module.
def determine_axis_roles(module):
    axis_roles = {}
    for p in module.ports:
        if is_axis_port(p) and not is_axis_role_found(axis_roles, p):
            (axis_interface, axis_port) = match_axis_regex(p)
            if axis_port in axis_slave_input_ports:
                axis_roles[axis_interface] = "slave" if p.mode == "input" else "master"
    return axis_roles

# Determines the AXI-MM roles for a given module.
def determine_aximm_roles(module):
    aximm_roles = {}
    for p in module.ports:
        if is_aximm_port(p) and not is_aximm_role_found(aximm_roles, p):
            (aximm_interface, aximm_port) = match_aximm_regex(p)
            if aximm_port in aximm_slave_input_ports:
                aximm_roles[aximm_interface] = "slave" if p.mode == "input" else "master"
    return aximm_roles

# Parses modules from a Verilog File
def get_modules_from_verilog_file(verilog_file_path):
    vlog_ex = vlog.VerilogExtractor()
    return vlog_ex.extract_objects(verilog_file_path)

# Main function for a single module
def generate_port_mappings_for_module(port_mapping_file, module, axis_roles, aximm_roles):
    warnings = False
    port_mapping_file.write("module {0}\n".format(module.name))
    for p in module.ports:
        if is_axis_port(p) and is_axis_role_found(axis_roles, p):
            (axis_interface, axis_port) = match_axis_regex(p)
            axis_role = axis_roles[axis_interface]
            port_mapping_file.write("axis {0} {1} axis_{2} {3}\n".format(axis_role, p.name, axis_interface, axis_port))
        elif is_aximm_port(p) and is_aximm_role_found(aximm_roles, p):
            (aximm_interface, aximm_port) = match_aximm_regex(p)
            aximm_role = aximm_roles[aximm_interface]
            port_mapping_file.write("aximm {0} {1} aximm_{2} {3}\n".format(aximm_role, p.name, aximm_interface, aximm_port))
        else:
            port_size = determine_port_width(p)
            if port_size == "?":
                print("WARNING: Width for Port", p.name, "could not be determined automatically. Please set this value manually.")
                warnings = True
            port_mapping_file.write("{0} {1} {2} {3}\n".format(p.mode, port_size, p.name, p.name))
    port_mapping_file.write("\n")
    print("Port Mappings for {0} has been added to the port map file.".format(module.name))
    return warnings

# Determines the modules in each file, and calls generate_port_mappings_for_module
def generate(design_folder, rtl_files, cmd_overwrite):
    modules_folder = design_folder / "modules"
    rtl_folder = modules_folder / "rtl"
    port_map_file_path = rtl_folder / "port.map"

    if not design_folder.exists():
        raise ValueError("The design folder does not exist.")
    if not design_folder.is_dir():
        raise ValueError("The design path specified is not a directory.")

    if port_map_file_path.exists():
        if not cmd_overwrite:
            overwrite = input("WARNING: Port Map File already exists. Do you want to overwrite this file? (Y/n)")
            if overwrite.upper() == "N" and overwrite:
                exit()

    with open(port_map_file_path, "w") as port_mapping_file:
        warnings = False
        for i in range(len(rtl_files)):
            rtl_file = rtl_files[i]
            rtl_file_path = rtl_folder / rtl_file
            if not rtl_file_path.exists():
                raise ValueError("The RTL file at {0} does not exist.".format(rtl_file_path))
            if vlog.is_verilog(rtl_file_path):
                modules = get_modules_from_verilog_file(rtl_file_path)
            else:
                print("ERROR: File {0} is not supported. Only Verilog/SystemVerilog files are supported.".format(rtl_file))
                exit()
            
            for m in modules:
                # Finds all the AXI-S roles (master/slave) for each AXI-S interface
                axis_roles = determine_axis_roles(m)
                aximm_roles = determine_aximm_roles(m)
                warnings = True if generate_port_mappings_for_module(port_mapping_file, m, axis_roles, aximm_roles) else warnings
        if warnings:
            print("WARNING: Successfully generated port mapping file with manual input required.")
            print("Please manually replace '?' with the correct values before running the wrapper generation script.")
        else:
            print("Successfully generated port mapping file with no warnings.")

if __name__ == "__main__":
    # Parse arguments
    parser = argparse.ArgumentParser(description='Generates a Port Mapping File for RTL => RAD-Sim Module generation.')
    parser.add_argument('path', metavar='path', type=Path,
                        help='the base file path of the design')
    parser.add_argument('rtl_files', metavar='design', nargs="+",
                        help='the rtl file name to generate a port mapping file for (ex. adder.v)')
    parser.add_argument('--overwrite', action='store_true',
                        help='overwrites any pre-existing port map files if it already exists')
    args = parser.parse_args()

    # Generate Port Mappings File
    generate(args.path, args.rtl_files, args.overwrite)
