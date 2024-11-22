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

class client : public RADSimModule {
private:
  // FIFO to store numbers
  fifo<sc_bv<DATAW>>* client_tdata_fifo;
  fifo<bool>* client_tlast_fifo;

  // Data FIFO signals
  sc_signal<sc_bv<DATAW>> client_tdata_fifo_rdata_signal;
  sc_signal<bool> client_tdata_fifo_wen_signal;
  sc_signal<bool> client_tdata_fifo_ren_signal;
  sc_signal<bool> client_tdata_fifo_full_signal;
  sc_signal<bool> client_tdata_fifo_empty_signal; 
  sc_signal<bool> client_tdata_fifo_almost_full_signal; 
  sc_signal<bool> client_tdata_fifo_almost_empty_signal;

  // Last FIFO signals
  sc_signal<bool> client_tlast_fifo_rdata_signal;
  sc_signal<bool> client_tlast_fifo_wen_signal;
  sc_signal<bool> client_tlast_fifo_ren_signal;
  sc_signal<bool> client_tlast_fifo_full_signal;
  sc_signal<bool> client_tlast_fifo_empty_signal; 
  sc_signal<bool> client_tlast_fifo_almost_full_signal; 
  sc_signal<bool> client_tlast_fifo_almost_empty_signal;

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
  RADSimDesignContext* radsim_design;

  client(const sc_module_name &name, RADSimDesignContext* radsim_design);
  ~client();

  void Assign(); // Combinational logic process
  void Tick();   // Sequential logic process
  SC_HAS_PROCESS(client);
  void RegisterModuleInfo();
};