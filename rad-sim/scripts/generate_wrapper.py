# Generate Wrapper Code for RTL Support
# Current version only supports AXI-S
# Arguments:
#   [1] => Path to the design folder
#   [2...] => Modules to generate wrapper code for
import argparse
from pathlib import Path

DEFAULT_PORT_WIDTH = 1024

port_type_translation = {
  "input": "sc_in",
  "output": "sc_out",
  "inout": "sc_inout"
}

def generate_source_wrapper(design_name, modules_folder, dataw, mappings, axis_roles):
    verilated_design = "V" + design_name
    design_inst = "v" + design_name

    with open(modules_folder / (design_name + ".cpp"), "w") as wrapper_cpp_file:
      wrapper_cpp_file.write("#include <" + design_name + ".hpp>\n\n")

      wrapper_cpp_file.write(design_name + "::" + design_name + "(const sc_module_name &name) : RADSimModule(name) {\n")

      wrapper_cpp_file.write("\t" + "char " + design_inst + "_name[25];\n")
      wrapper_cpp_file.write("\tstd::string " + design_inst + "_name_str = std::string(name) + \"_vmvm\";\n")
      wrapper_cpp_file.write("\tstd::strcpy(" + design_inst + "_name, " + design_inst + "_name_str.c_str());\n\n")
      wrapper_cpp_file.write("\t" + design_inst + " = new " + verilated_design + "{" + design_inst + "_name};\n")

      #inputs and outputs connections
      if not design_name in mappings:
        print("WARNING: No mappings declared for the module", design_name)
      elif len(mappings[design_name]) == 0:
        print("WARNING: Empty mappings for the module", design_name)
      else:
        for port in mappings[design_name]:
          wrapper_cpp_file.write("\t" + design_inst + "->" + port[2] + "(" + port[3] + ");\n")

      if axis_roles != None:
        wrapper_cpp_file.write("\n\tthis->RegisterModuleInfo();\n")
      wrapper_cpp_file.write("}\n\n")

      wrapper_cpp_file.write(design_name + "::~" + design_name + "() {\n")
      wrapper_cpp_file.write("\t" + design_inst + "->final();\n")
      wrapper_cpp_file.write("\tdelete " + design_inst + ";\n")
      wrapper_cpp_file.write("}\n\n")

      if axis_roles != None:
        wrapper_cpp_file.write("void " + design_name + "::RegisterModuleInfo() {\n")
        wrapper_cpp_file.write("\tstd::string port_name;\n")
        wrapper_cpp_file.write("\t_num_noc_axis_slave_ports = 0;\n")
        wrapper_cpp_file.write("\t_num_noc_axis_master_ports = 0;\n")
        wrapper_cpp_file.write("\t_num_noc_aximm_slave_ports = 0;\n")
        wrapper_cpp_file.write("\t_num_noc_aximm_master_ports = 0;\n\n")

        for axis_interface, axis_role in axis_roles.items():
          wrapper_cpp_file.write("\tport_name = module_name + \"." + axis_interface + "\";\n")
          if axis_role == "master":
              wrapper_cpp_file.write("\tRegisterAxisMasterPort(port_name, &" + axis_interface + ", " + dataw + ", 0);\n\n")
          else:
              wrapper_cpp_file.write("\tRegisterAxisSlavePort(port_name, &" + axis_interface + ", " + dataw + ", 0);\n\n")
        
        wrapper_cpp_file.write("}\n")
      else:
        print("WARNING: Module {0} is not connected to the NOC via AXI-S.".format(design_name))

def generate_header_wrapper(design_name, modules_folder, mappings, axis_roles):
    verilated_design = "V" + design_name
    design_inst = "v" + design_name

    with open(modules_folder / (design_name + ".hpp"), "w") as wrapper_hpp_file:
      wrapper_hpp_file.write("#pragma once\n\n")
      wrapper_hpp_file.write("#include <axis_interface.hpp>\n")
      wrapper_hpp_file.write("#include <design_context.hpp>\n")
      wrapper_hpp_file.write("#include <radsim_defines.hpp>\n")
      wrapper_hpp_file.write("#include <radsim_module.hpp>\n")
      wrapper_hpp_file.write("#include <string>\n")
      wrapper_hpp_file.write("#include <systemc.h>\n")
      wrapper_hpp_file.write("#include <vector>\n\n")
      wrapper_hpp_file.write("#include <" + verilated_design + ".h>\n")

      wrapper_hpp_file.write("class " + design_name + " : " + "public RADSimModule {\n")
      wrapper_hpp_file.write("private:\n")
      wrapper_hpp_file.write("\t" + verilated_design + "* " + design_inst + ";\n\n")
      wrapper_hpp_file.write("public:\n")

      #inputs and outputs
      if not design_name in mappings:
        print("WARNING: No mappings declared for the module", design_name)
      elif len(mappings[design_name]) == 0:
        print("WARNING: Empty mappings for the module", design_name)
      else:
        for port in mappings[design_name]:
          if port[0] == "axis": continue # NoC connectors are handled separately
          if port[3] == "clk": continue # Clock connection provided by RADSimModule
          port_size_type = "bool" if port[1] == "1" else "sc_bv<" + port[1] + ">"
          wrapper_hpp_file.write("\t" + port[0] + "<" + port_size_type + "> " + port[3] + ";\n")

      #NoC connection, TODO: add support for AXI-MM
      if axis_roles != None:
        wrapper_hpp_file.write("\n")
        for axis_interface, axis_role in axis_roles.items():
          if axis_role == "master":
              wrapper_hpp_file.write("\taxis_master_port " + axis_interface + ";\n")
          else:
              wrapper_hpp_file.write("\taxis_slave_port " + axis_interface + ";\n")

      wrapper_hpp_file.write("\n\t" + design_name + "(const sc_module_name &name);\n")
      wrapper_hpp_file.write("\t~" + design_name + "();\n\n")

      wrapper_hpp_file.write("\tSC_HAS_PROCESS(" + design_name + ");\n")
      if axis_roles != None:
        wrapper_hpp_file.write("\tvoid RegisterModuleInfo();\n")
      else:
        print("WARNING: Module {0} is not connected to the NOC via AXI-S.".format(design_name))
      wrapper_hpp_file.write("};\n")

def read_port_mappings(port_mapping_file):
  current_module = ""
  mappings = {}
  axis_roles = {}
  with open(port_mapping_file) as pmf:
    for line in pmf:
      components = line.split()

      if not components: continue

      if components[0] == "module":
        if len(components) != 2: raise ValueError("A line specifying a module can only contain 2 parameters separated by a whitespace.")
        current_module = components[1]
        mappings[current_module] = [] # insert new dictionary entry
        print("Found port mappings for module", current_module)
      elif components[0] == "axis":
        if not current_module: raise ValueError("A module must be specified before mappings for the module.")
        if len(components) != 5: raise ValueError("Each line specifying an AXI-S port must contain 5 parameters separated by a whitespace.")

        (keyword, axis_role, rtl_port, axis_interface, axis_port) = components
        radsim_port = axis_interface + "." + axis_port
        mappings[current_module].append((keyword, axis_role, rtl_port, radsim_port))
        if current_module not in axis_roles:
          axis_roles[current_module] = {}
        if axis_interface not in axis_roles[current_module]:
          axis_roles[current_module][axis_interface] = axis_role
        else:
          if axis_roles[current_module][axis_interface] != axis_role:
            raise ValueError("Inconsistent AXI-S role for interface " + axis_interface + ". Each interface can either be master or slave.")
      else:
        port_mode = components[0]
        port_width = components[1]
        if not current_module: raise ValueError("A module must be specified before mappings for the module.")
        if len(components) != 4: raise ValueError("Each line specifying a port can only contain 4 parameters separated by a whitespace.")
        if port_mode != "input" and port_mode != "output" and port_mode != "inout": raise ValueError("The first argument of each port must be either axis/input/output/inout.")
        if port_width == "?": raise ValueError("Automated port mapping failed for " + components[2] + ". Manually add the port size.")
        if not port_width.isnumeric(): raise ValueError("Port Width must be a number.")

        port_mode = port_type_translation[port_mode] # translate input/output/inout to systemC types
        mappings[current_module].append((port_mode, port_width, components[2], components[3]))
  return (mappings, axis_roles)

def generate(design_folder, design_modules):
  modules_folder = design_folder / "modules"
  rtl_folder = modules_folder / "rtl"
  port_map_file_path = rtl_folder / "port.map"

  if not design_folder.exists():
      raise ValueError("The design folder does not exist.")
  if not design_folder.is_dir():
      raise ValueError("The design path specified is not a directory.")
  if not port_map_file_path.exists():
      raise ValueError("The port mapping file does not exist.")

  print("Reading Port Mappings...")
  mappings, axis_roles = read_port_mappings(port_map_file_path)
  print("Read Port Mappings Sucessfully!")
  for i in range(len(design_modules)):
    design_name = design_modules[i]
    #TODO: only ask for AXI-S data width if module contains AXI port.
    dataw = input("Enter the AXI-S data width for module " + design_name + " (default: " + str(DEFAULT_PORT_WIDTH) + "): ")
    dataw = dataw if dataw else str(DEFAULT_PORT_WIDTH)
    generate_source_wrapper(design_name, modules_folder, dataw, mappings, axis_roles.get(design_name))
    print("Generated Source Wrapper for module", design_name)
    generate_header_wrapper(design_name, modules_folder, mappings, axis_roles.get(design_name))
    print("Generated Header Wrapper for module", design_name)

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Generates RAD-Sim Module Wrapper files from a port mapping file.')
  parser.add_argument('path', metavar='path', type=Path,
                      help='the base file path of the design')
  parser.add_argument('design_modules', metavar='module', nargs="+",
                      help='the design module to generate wrapper for (ex. adder)')
  args = parser.parse_args()

  generate(args.path, args.design_modules)

