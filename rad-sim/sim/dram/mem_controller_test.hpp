#pragma once

#include <systemc.h>

#include <aximm_interface.hpp>
#include <radsim_utils.hpp>
#include <design_context.hpp>
#include <mem_controller.hpp>
#include <vector>
#include <random>
#include <bitset>
#include <cmath>

class mem_controller_test : public sc_module {
 private:
  std::string module_name;
  unsigned int _burst_size, _num_channels, _num_used_channels, _mem_capacity, _bl_bits;
  double _clk_period;
  unsigned int _num_write_cmds, _num_read_cmds;
  std::vector<unsigned int> _w_count, _aw_count, _b_count, _r_count, _ar_count;
  std::vector<std::vector<uint64_t>> _waddr, _raddr;
  std::vector<std::vector<sc_bv<AXI_MAX_DATAW>>> _wdata;
  std::vector<std::vector<unsigned int>> _read_issue_cycles, _read_latency;
  bool r_done, b_done;

 public:
  sc_in<bool> clk;
  sc_out<bool> rst;
  sc_vector<aximm_master_port> tx_interface;

  mem_controller_test(const sc_module_name& name, unsigned int num_cmds, 
    unsigned int test_mode, unsigned int burst_size, unsigned int num_channels,
    unsigned int mem_capacity_mb, unsigned int num_used_channels, 
    unsigned int addressable_word_size_bytes, double clk_peiod);
  ~mem_controller_test();

  void aw_source();
  void w_source();
  void ar_source();
  void b_sink();
  void r_sink();
  void assign();
  SC_HAS_PROCESS(mem_controller_test);
};

class mem_controller_system : public sc_module {
 private:
  sc_signal<bool> rst_sig;
  sc_vector<aximm_signal> test_to_dut;

 public:
  mem_controller_test* test_inst;
  mem_controller* dut_inst;
  sc_clock* clk_sig;
  sc_clock* mem_clk_sig;

  mem_controller_system(const sc_module_name& name);
  ~mem_controller_system();
};