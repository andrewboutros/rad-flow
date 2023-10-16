# Port Mapping Automated Parser
import argparse
import re
import verilog_parser as vlog
from pathlib import Path

verilog_range_regex = "\[(\d*):(\d*)\]"
verilog_axis_regex = "axis_{0}_interface_(.*)"

def determinePortWidth(port):
    if not port.data_type:
        return 1
    
    port_properties = port.data_type.split()
    # explicitly define test cases, default behavior if no cases match is to output ?
    if len(port_properties) == 1:
        indices = re.match(verilog_range_regex, port_properties[0])
        if port_properties[0] == "reg":
            return 1
        elif indices and len(indices.groups()) == 2:
            return int(indices.groups()[0]) - int(indices.groups()[1]) + 1
    elif len(port_properties) == 2:
        indices = re.match(verilog_range_regex, port_properties[1])
        if indices and len(indices.groups()) == 2:
            return int(indices.groups()[0]) - int(indices.groups()[1]) + 1
    
    return "?"

def isAxisPort(port, module):
    if re.match(verilog_axis_regex.format(module), port.name):
        return True
    return False

def isAxisRoleMaster(module):
    for p in module.ports:
        if isAxisPort(p, module.name):
            signal = re.match(verilog_axis_regex.format(module.name), p.name)
            if signal.groups()[0] == "tdata":
                return 0 if p.mode == "input" else 1
    return None # If no AXI-S data port is found

def getModulesFromVerilogFile(verilog_file_path):
    vlog_ex = vlog.VerilogExtractor()
    return vlog_ex.extract_objects(verilog_file_path)

def generatePortMappingsForModule(port_mapping_file, module):
    warnings = False
    port_mapping_file.write("module {0}\n".format(module.name))
    is_master = isAxisRoleMaster(module) # Scan all ports to determine whether AXI-S interface is master or slave
    for p in module.ports:
        if isAxisPort(p, module.name):
            signal = re.match(verilog_axis_regex.format(module.name), p.name)
            axis_role = "master" if is_master else "slave"
            port_mapping_file.write("axis {0} {1} axis_{2}_interface.{3}\n".format(axis_role, p.name, module.name, signal.groups()[0]))
        else:
            port_size = determinePortWidth(p)
            if port_size == "?":
                print("WARNING: Width for Port", p.name, "could not be determined automatically. Please set this value manually.")
                warnings = True
            port_mapping_file.write("{0} {1} {2} {3}\n".format(p.mode, port_size, p.name, p.name))
    port_mapping_file.write("\n")
    print("Port Mappings for {0} has been added to the port map file.".format(module.name))
    return warnings

parser = argparse.ArgumentParser(description='Generates a Port Mapping File for RTL => RAD-Sim Module generation.')
parser.add_argument('path', metavar='path', type=Path,
                    help='the base file path of the design')
parser.add_argument('rtl_files', metavar='design', nargs="+",
                    help='the rtl file name to generate a port mapping file for (ex. adder.v)')
parser.add_argument('--overwrite', action='store_true',
                    help='overwrites any pre-existing port map files if it already exists')
args = parser.parse_args()

design_folder = args.path
modules_folder = design_folder / "modules"
rtl_folder = modules_folder / "rtl"
port_map_file_path = rtl_folder / "port.map"

if not design_folder.exists():
    raise ValueError("The design folder does not exist.")
if not design_folder.is_dir():
    raise ValueError("The design path specified is not a directory.")

if port_map_file_path.exists():
    if not args.overwrite:
        overwrite = input("WARNING: Port Map File already exists. Do you want to overwrite this file? (Y/n)")
        if overwrite.upper() == "N" and overwrite:
            exit()

with open(port_map_file_path, "w") as port_mapping_file:
    warnings = False
    for i in range(len(args.rtl_files)):
        rtl_file = args.rtl_files[i]
        rtl_file_path = rtl_folder / rtl_file
        if not rtl_file_path.exists():
            raise ValueError("The RTL file at {0} does not exist.".format(rtl_file_path))
        if vlog.is_verilog(rtl_file_path):
            modules = getModulesFromVerilogFile(rtl_file_path)
        else:
            print("ERROR: File {0} is not supported. Only Verilog/SystemVerilog files are supported.".format(rtl_file))
            exit()
        for m in modules:
            warnings = True if generatePortMappingsForModule(port_mapping_file, m) else warnings
    if warnings:
        print("WARNING: Successfully generated port mapping file with manual input required.")
        print("Please manually replace '?' with the correct values before running the wrapper generation script.")
    else:
        print("Successfully generated port mapping file with no warnings.")