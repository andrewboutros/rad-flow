#pragma once

#include <axis_interface.hpp>
#include <design_context.hpp>
#include <queue>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>

#define DATAW 128

class client_wrapper : public radsim_module {
private:

public:
  sc_in<bool> rst;
  sc_in<sc_bv<DATAW>> client_tdata;
  sc_in<bool> client_tlast;
  sc_in<bool> client_valid;
  sc_out<bool> client_ready;
  // Interface to the NoC
  axis_master_port axis_client_interface;

  client_wrapper(const sc_module_name &name);
  ~client_wrapper();
  
  SC_HAS_PROCESS(client_wrapper);
  void RegisterModuleInfo();
};