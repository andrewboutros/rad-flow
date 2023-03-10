#pragma once

#include <systemc.h>

#include <axis_interface.hpp>
#include <axis_master_adapter.hpp>
#include <axis_slave_adapter.hpp>
#include <radsim_utils.hpp>
#include <design_context.hpp>
#include <sstream>
#include <functional>
#include <unordered_map>
#include <queue>

#include <memory_system.h>

class dram_axis : public sc_module {
 private:
  std::string module_name;
  dramsim3::MemorySystem _mem;
  std::unordered_map<uint64_t, sc_bv<AXIS_MAX_DATAW>> _mem_contents;
  std::queue<std::pair<uint64_t, sc_bv<AXIS_MAX_DATAW>>> _read_resp_queue;
  sc_fifo<uint64_t>* _rx_requests_addr;
  sc_fifo<bool>* _rx_requests_type;

  sc_signal<uint64_t> num_push, num_pop;
  uint64_t rx_req_addr;
  bool rx_req_type;
  sc_signal<bool> rx_req_keep;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  axis_master_port tx_interface;
  axis_slave_port rx_interface;

  dram_axis(const sc_module_name& name, const std::string& config_file, 
    const std::string& output_dir);
  ~dram_axis();
  
  void MemReadCallback(uint64_t addr);
  void MemWriteCallback(uint64_t addr);
  void PrintStats();

  void Assign();
  void Tick();
  SC_HAS_PROCESS(dram_axis);

  //void RegisterModuleInfo();
};
