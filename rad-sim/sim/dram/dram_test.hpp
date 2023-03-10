#pragma once

#include <systemc.h>

#include <axis_interface.hpp>
#include <axis_master_adapter.hpp>
#include <axis_slave_adapter.hpp>
#include <radsim_utils.hpp>
#include <design_context.hpp>

#include <dram_axis.hpp>

#define NUM_READ_CMDS 3
#define NUM_WRITE_CMDS 3

class dram_test : public sc_module {
 private:
  std::string module_name;

 public:
  sc_in<bool> clk;
  sc_out<bool> rst;
  axis_master_port tx_interface;
  axis_slave_port rx_interface;

  dram_test(const sc_module_name& name);
  ~dram_test();

  void source();
  void sink();
  void assign();
  SC_HAS_PROCESS(dram_test);
};

class dram_system : public sc_module {
 private:
  sc_signal<bool> rst_sig;
  axis_signal test_to_dut;
  axis_signal dut_to_test;

 public:
  dram_test* test_inst;
  dram_axis* dut_inst;
  sc_clock* clk_sig;

  dram_system(const sc_module_name& name);
  ~dram_system();
};