#pragma once

#include <radsim_config.hpp>
#include <mult.hpp>
#include <client_mult.hpp>
#include <portal_mult.hpp> //AKB ADDED
#include <systemc.h>
#include <vector>
#include <design_top.hpp> //AKB ADDED

class mult_top : public design_top {
private:
  mult *mult_inst;
  client_mult *client_inst;
  portal_mult *portal_inst; //AKB added

public:
  sc_in<bool> rst;
  // Client's interface
  sc_in<sc_bv<DATAW>> client_tdata;
  sc_in<bool> client_tlast;
  sc_in<bool> client_valid;
  sc_out<bool> client_ready;
  sc_out<sc_bv<DATAW>> response;
  sc_out<bool> response_valid;
  //AKB ADDED for portal module:
  //sc_in<bool> portal_in;
  //sc_out<bool> portal_out;

  mult_top(const sc_module_name &name, RADSimDesignContext* radsim_design); //AKB ADDED last arg
  ~mult_top();
};