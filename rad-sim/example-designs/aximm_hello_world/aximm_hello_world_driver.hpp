#pragma once

#include <chrono>
#include <radsim_config.hpp>
#include <requester.hpp>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>
#include <vector>

struct Command {
  unsigned int cycle;
  bool type;
  uint64_t addr;
  sc_bv<DATAW> data;
};

class aximm_hello_world_driver : public sc_module {
private:
  std::vector<Command> hello_world_sim_trace;

public:
  sc_in<bool> clk;
  sc_out<bool> rst;
  sc_out<uint64_t> req_addr;
  sc_out<bool> req_type;
  sc_out<sc_bv<DATAW>> req_wdata;
  sc_out<bool> req_valid;
  sc_in<bool> req_ready;
  sc_in<unsigned int> received_responses;

  aximm_hello_world_driver(const sc_module_name &name);
  ~aximm_hello_world_driver();

  void source();
  void sink();

  SC_HAS_PROCESS(aximm_hello_world_driver);
};