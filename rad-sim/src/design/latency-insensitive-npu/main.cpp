#include <systemc.h>

#include <fstream>
#include <iostream>
#include <npu_tb.hpp>

npu_system* npu_top;
std::unordered_map<std::string, unsigned int> module_assignment;

void SetModuleAssignment() {
  std::string module_name;
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    module_name = "axis_mvu_sector_" + std::to_string(sector_id);
    module_assignment[module_name] = sector_id;
  }

  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
      module_name = "axis_evrf_" + std::to_string(thread_id) + "_" + std::to_string(sector_id);
      module_assignment[module_name] = SECTORS + (thread_id * (3 * SECTORS + 1)) + (sector_id * 3);
      module_name = "axis_mfu0_" + std::to_string(thread_id) + "_" + std::to_string(sector_id);
      module_assignment[module_name] = SECTORS + (thread_id * (3 * SECTORS + 1)) + (sector_id * 3) + 1;
      module_name = "axis_mfu1_" + std::to_string(thread_id) + "_" + std::to_string(sector_id);
      module_assignment[module_name] = SECTORS + (thread_id * (3 * SECTORS + 1)) + (sector_id * 3) + 2;
    }
    module_name = "axis_ld_" + std::to_string(thread_id);
    module_assignment[module_name] = SECTORS + (thread_id * 3 * SECTORS) + 1;
  }

  module_name = "axis_inst_dispatcher";
  module_assignment[module_name] = SECTORS + (THREADS * (3 * SECTORS + 1));

  cout << "Module Assignment:" << endl;
  for (auto itr = module_assignment.begin(); itr != module_assignment.end(); itr++) {
    cout << itr->first << " @ node " << itr->second << endl;
  }
  cout << "*********************************" << endl;
}

int sc_main(int argc, char* argv[]) {
  SetModuleAssignment();
  npu_top = new npu_system("npu");

  sc_start();

  return 0;
}