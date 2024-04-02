#pragma once

#include <axis_interface.hpp>
#include <design_context.hpp>
#include <queue>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>
#include <radsim_utils.hpp>
#include <fifo.hpp>

#define FIFO_DEPTH 16

class client_mult : public RADSimModule {
private:
  std::queue<sc_bv<DATAW>> client_tdata_fifo; // FIFO to store numbers
  unsigned int client_fifo_depth;    // MAXIMUM number of factors to store in FIFO
  sc_signal<bool> client_fifo_full;  // Signal flagging factor FIFO is full
  bool testbench_tlast;

public:
  sc_in<bool> rst;
  // Interface to driver logic
  sc_in<sc_bv<DATAW>> client_tdata;
  sc_in<bool> client_tlast;
  sc_in<bool> client_valid;
  sc_out<bool> client_ready;
  // Interface to the NoC
  axis_master_port axis_client_interface;
  //AKB added bc used in functions outside of constructor:
  RADSimDesignContext* radsim_design;

  client_mult(const sc_module_name &name, unsigned int fifo_depth, RADSimDesignContext* radsim_design); //AKB added last arg
  ~client_mult();

  void Assign(); // Combinational logic process
  void Tick();   // Sequential logic process
  SC_HAS_PROCESS(client_mult);
  void RegisterModuleInfo();
};