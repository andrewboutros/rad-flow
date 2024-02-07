#pragma once

#include <radsim_config.hpp>
#include <adder.hpp>
#include <client.hpp>
#include <portal.hpp> //AKB ADDED
#include <systemc.h>
#include <vector>
#include <design_top.hpp> //AKB ADDED

class add_top : public design_top {
//class add_top : public sc_module {
private:
  adder *adder_inst;
  client *client_inst;
  portal *portal_inst; //AKB added

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
  sc_out<bool> portal_recvd;

  add_top(const sc_module_name &name, RADSimDesignContext* radsim_design); //AKB ADDED last arg
  ~add_top();
};