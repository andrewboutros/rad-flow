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

class mult : public RADSimModule {
private:
  sc_bv<DATAW> mult_rolling_product; // Product to store result
  sc_signal<bool> t_finished; // Signal flagging that the transaction has terminated

public:
  RADSimDesignContext* radsim_design;
  sc_in<bool> rst;
  sc_out<bool> response_valid;
  sc_out<sc_bv<DATAW>> response;
  // Interface to the NoC
  axis_slave_port axis_mult_interface;

  mult(const sc_module_name &name, RADSimDesignContext* radsim_design);
  ~mult();

  void Assign(); // Combinational logic process
  void Tick();   // Sequential logic process
  SC_HAS_PROCESS(mult);
  void RegisterModuleInfo();
};