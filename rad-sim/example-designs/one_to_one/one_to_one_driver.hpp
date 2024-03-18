#pragma once

#include <chrono>
#include <radsim_config.hpp>
#include <traffic_generator.hpp>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>
#include <queue>

struct Command {
  unsigned int cycle;
  bool type;
  uint64_t addr;
  sc_bv<DATAW> data;
};

class one_to_one_driver : public sc_module {
private:
  std::vector<Command> one_to_one_sim_trace;
  
public:
  sc_in<bool> clk;
  sc_out<bool> rst;
  sc_out<uint64_t> req_addr;
  sc_out<bool> req_type;
  sc_out<sc_bv<DATAW>> req_wdata;
  sc_out<bool> req_valid;
  sc_in<bool> req_ready;
  sc_in<unsigned int> received_responses;
  int start_cycle, end_cycle;
  std::chrono::steady_clock::time_point start_time, end_time;

  one_to_one_driver(const sc_module_name &name);
  ~one_to_one_driver();

  void source();
  void sink();

  SC_HAS_PROCESS(one_to_one_driver);
};