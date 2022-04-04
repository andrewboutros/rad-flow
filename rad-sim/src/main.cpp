#include <systemc.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <radsim_config.hpp>
#include <design_tb.hpp>

std::ofstream* radsim_log;
std::ostream* gWatchOut;
std::unordered_map<std::string, unsigned int> module_assignment;

design_tb* npu_system;

void SetModuleAssignment() {
  std::string module_assignment_filename = "fpga_chiplet_2.place";
  std::string module_assignment_path = RADSIM_SRC_ROOTDIR + module_assignment_filename;

  std::ifstream module_assignment_file(module_assignment_path);

  std::string line;
  while (std::getline(module_assignment_file, line)) {
    std::stringstream ss(line);
    std::string module_name;
    std::string module_placement_str;
    //Read each comma delimited string and convert to required type
    std::getline(ss, module_name, ' ');
    std::getline(ss, module_placement_str, ' ');
    unsigned int module_placement = std::stoi(module_placement_str);
    module_assignment[module_name] = module_placement;
  }

  cout << "Module Assignment:" << endl;
  for (auto itr = module_assignment.begin(); itr != module_assignment.end(); itr++) {
    cout << itr->first << " @ node " << itr->second << endl;
  }
  cout << "*********************************" << endl;
}

void ParseCrossroadsConfig() {
  RADSimConfig::AddIntKnob("noc_adapter_fifo_depth", 16);
  RADSimConfig::AddIntKnob("noc_adapter_output_fifo_depth", 2);
  RADSimConfig::AddIntKnob("num_nodes", 100);
  RADSimConfig::AddIntKnob("num_virtual_channels", 5);
}

int sc_main(int argc, char* argv[]) {
  ParseCrossroadsConfig();
  SetModuleAssignment();
  std::string log_file = "/Users/andrew/PhD/dev/rad-sim-opt-npu-multithread-hard-c2/sim_log";
  std::string noc_config_file = "/Users/andrew/PhD/dev/rad-sim-opt-npu-multithread-hard-c2/src/noc_config";

  radsim_log = new ofstream(log_file, std::ofstream::out);
  gWatchOut = &cout;

  npu_system = new design_tb("rad_npu_system", noc_config_file);
  sc_start();

  return 0;
}