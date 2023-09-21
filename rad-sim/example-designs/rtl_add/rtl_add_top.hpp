#pragma once

#include <radsim_config.hpp>
#include <adder_wrapper.hpp>
#include <client_wrapper.hpp>
#include <systemc.h>
#include <vector>

class rtl_add_top : public sc_module {
private:
  adder_wrapper *adder_inst;
  client_wrapper *client_inst;

public:
  sc_in<bool> rst;
  // Client's interface
  sc_in<sc_bv<DATAW>> client_tdata;
  sc_in<bool> client_tlast;
  sc_in<bool> client_valid;
  sc_out<bool> client_ready;

  rtl_add_top(const sc_module_name &name);
  ~rtl_add_top();
};