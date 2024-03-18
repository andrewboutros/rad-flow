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

class two_to_two_driver : public sc_module {
private:
  std::vector<Command> two_to_two_sim_trace;
  
public:
  sc_in<bool> clk;

  sc_out<bool> rst;
  sc_out<uint64_t> req_addr;
  sc_out<bool> req_type;
  sc_out<sc_bv<DATAW>> req_wdata;
  sc_out<bool> req_valid;
  sc_in<bool> req_ready;
  sc_in<unsigned int> received_responses;

  sc_out<bool> rst_2;
  sc_out<uint64_t> req_addr_2;
  sc_out<bool> req_type_2;
  sc_out<sc_bv<DATAW>> req_wdata_2;
  sc_out<bool> req_valid_2;
  sc_in<bool> req_ready_2;
  sc_in<unsigned int> received_responses_2;


  int start_cycle, end_cycle;
  std::chrono::steady_clock::time_point start_time, end_time;

  two_to_two_driver(const sc_module_name &name);
  ~two_to_two_driver();

  void source();
  void sink();

  SC_HAS_PROCESS(two_to_two_driver);
};