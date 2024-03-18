#pragma once

#include <radsim_config.hpp>
#include <traffic_generator.hpp>
#include <systemc.h>
#include <mem_controller.hpp>
#include <vector>

class two_to_two_top : public sc_module {
private:
  traffic_generator *traffic_generator_inst_0;
  traffic_generator *traffic_generator_inst_1;
  std::vector<mem_controller *> ext_mem;
  std::vector<sc_clock *> mem_clks;

public:
  sc_in<bool> rst;
  // Requester's interface
  sc_in<uint64_t> req_addr;
  sc_in<bool> req_type;
  sc_in<sc_bv<DATAW>> req_wdata;
  sc_in<bool> req_valid;
  sc_out<bool> req_ready;
  sc_out<unsigned int> received_responses;

  sc_in<bool> rst2;
  // Requester's interface
  sc_in<uint64_t> req_addr2;
  sc_in<bool> req_type2;
  sc_in<sc_bv<DATAW>> req_wdata2;
  sc_in<bool> req_valid2;
  sc_out<bool> req_ready2;
  sc_out<unsigned int> received_responses2;

  two_to_two_top(const sc_module_name &name);
  ~two_to_two_top();
};