#pragma once

#include <radsim_config.hpp>
#include <traffic_generator.hpp>
#include <systemc.h>
#include <mem_controller.hpp>
#include <vector>

class one_to_one_top : public sc_module {
private:
  traffic_generator *traffic_generator_inst;
  mem_controller * ext_mem;
  sc_clock * mem_clks;

public:
  sc_in<bool> rst;
  // Requester's interface
  sc_in<uint64_t> req_addr;
  sc_in<bool> req_type;
  sc_in<sc_bv<DATAW>> req_wdata;
  sc_in<bool> req_valid;
  sc_out<bool> req_ready;
  sc_out<unsigned int> received_responses;

  one_to_one_top(const sc_module_name &name);
  ~one_to_one_top();
};