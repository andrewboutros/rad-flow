#pragma once

#include <radsim_config.hpp>
#include <adder.hpp>
#include <client.hpp>
#include <systemc.h>
#include <vector>

#define DATAW 128

class rtl_add_top : public sc_module {
private:
  adder *adder_inst;
  client *client_inst;

public:
  sc_in<bool> rst;
  // Client's interface
  sc_in<sc_bv<DATAW>> client_tdata;
  sc_in<bool> client_tlast;
  sc_in<bool> client_valid;
  sc_out<bool> client_ready;

  // Adder's interface
  sc_out<sc_bv<DATAW>> response;
  sc_out<bool> response_valid;

  rtl_add_top(const sc_module_name &name);
  ~rtl_add_top();
};