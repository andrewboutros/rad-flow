#pragma once

#include <radsim_config.hpp>
#include <adder.hpp>
#include <client.hpp>
#include <systemc.h>
#include <vector>

class add_top : public sc_module {
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
  sc_out<sc_bv<DATAW>> response;
  sc_out<bool> response_valid;

  add_top(const sc_module_name &name, RADSimDesignContext* radsim_design); //AKB ADDED last arg
  ~add_top();
};