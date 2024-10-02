#pragma once

#include <axis_interface.hpp>
#include <design_context.hpp>
#include <queue>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <client.hpp>
#include <string>
#include <systemc.h>
#include <vector>
#include <radsim_utils.hpp>
#include <tuple>
#include <fifo.hpp>

#define FIFO_DEPTH 16

class adder : public RADSimModule {
private:
  sc_bv<DATAW> adder_rolling_sum; // Sum to store result
  sc_signal<bool> t_finished; // Signal flagging that the transaction has terminated
  //std::queue<std::tuple<sc_bv<DATAW>, bool>> adder_tdata_tlast_fifo;
  // FIFO to store numbers
  fifo<sc_bv<DATAW>>* adder_tdata_fifo;
  fifo<bool>* adder_tlast_fifo;

  // Data FIFO signals
  sc_signal<sc_bv<DATAW>> adder_tdata_fifo_rdata_signal;
  sc_signal<bool> adder_tdata_fifo_wen_signal;
  sc_signal<bool> adder_tdata_fifo_ren_signal;
  sc_signal<bool> adder_tdata_fifo_full_signal;
  sc_signal<bool> adder_tdata_fifo_empty_signal; 
  sc_signal<bool> adder_tdata_fifo_almost_full_signal; 
  sc_signal<bool> adder_tdata_fifo_almost_empty_signal;

  // Last FIFO signals
  sc_signal<bool> adder_tlast_fifo_rdata_signal;
  sc_signal<bool> adder_tlast_fifo_wen_signal;
  sc_signal<bool> adder_tlast_fifo_ren_signal;
  sc_signal<bool> adder_tlast_fifo_full_signal;
  sc_signal<bool> adder_tlast_fifo_empty_signal; 
  sc_signal<bool> adder_tlast_fifo_almost_full_signal; 
  sc_signal<bool> adder_tlast_fifo_almost_empty_signal;

  bool testbench_tlast;

public:
  RADSimDesignContext* radsim_design;
  sc_in<bool> rst;
  sc_out<bool> response_valid;
  sc_out<sc_bv<DATAW>> response;
  // Interface to the NoC
  axis_slave_port axis_adder_interface;
  axis_master_port axis_adder_master_interface;
  //to connect to fifo
  //sc_in<sc_bv<DATAW>> adder_tdata;
  sc_signal<sc_bv<DATAW>> adder_tdata;
  //sc_in<bool> adder_tlast;

  adder(const sc_module_name &name, RADSimDesignContext* radsim_design); //AKB added last arg
  ~adder();

  void Assign(); // Combinational logic process
  void Tick();   // Sequential logic process
  SC_HAS_PROCESS(adder);
  void RegisterModuleInfo();
};