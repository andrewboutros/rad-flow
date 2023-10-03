#pragma once

#include <axis_interface.hpp>
#include <design_context.hpp>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>
#include <client_wrapper.hpp>

class adder_wrapper : public radsim_module {
private:
  sc_bv<DATAW> adder_rolling_sum; // Sum to store result
  sc_signal<bool> t_finished; // Signal flagging that the transaction has terminated

public:
  sc_in<bool> rst;
  sc_out<bool> response_valid;
  sc_out<sc_bv<DATAW>> response;
  // Interface to the NoC
  axis_slave_port axis_adder_interface;

  adder_wrapper(const sc_module_name &name);
  ~adder_wrapper();

  SC_HAS_PROCESS(adder_wrapper);
  void RegisterModuleInfo();
};