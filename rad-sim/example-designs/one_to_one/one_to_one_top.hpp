#pragma once

#include <radsim_config.hpp>
#include <traffic_generator.hpp>
#include <systemc.h>
#include <vector>

class one_to_one_top : public sc_module {
private:
  traffic_generator *receiver_inst;
  traffic_generator *generator_inst;

public:
  sc_in<bool> rst;
  // generator's interface
  sc_in<sc_bv<DATAW>> generator_tdata;
  sc_in<bool> generator_tlast;
  sc_in<bool> generator_valid;
  sc_out<bool> generator_ready;
  sc_out<sc_bv<DATAW>> response;
  sc_out<bool> response_valid;

  one_to_one_top(const sc_module_name &name);
  ~one_to_one_top();
};