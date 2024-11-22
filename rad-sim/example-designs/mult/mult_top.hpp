#pragma once

#include <radsim_config.hpp>
#include <mult.hpp>
#include <client.hpp>
#include <systemc.h>
#include <vector>
#include <design_top.hpp>
#include <radsim_module.hpp>

class mult_top : public RADSimDesignTop {
private:
  mult *mult_inst;
  client *client_inst;
  RADSimDesignContext* radsim_design;

public:
  sc_in<bool> rst;
  // Client's interface
  sc_in<sc_bv<DATAW>> client_tdata;
  sc_in<bool> client_tlast;
  sc_in<bool> client_valid;
  sc_out<bool> client_ready;
  sc_out<sc_bv<DATAW>> response;
  sc_out<bool> response_valid;

  mult_top(const sc_module_name &name, RADSimDesignContext* radsim_design);
  ~mult_top();
};