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
  int16_t input_value;
};

class accum_multiply_driver : public sc_module {
private:
  std::vector<Command> accum_multiply_sim_trace;

public:
  sc_in<bool> clk;
  sc_out<bool> rst;
  sc_out<int16_t> source_wdata;
  sc_out<bool> source_valid;
  sc_in<bool> source_ready;
  sc_in<int16_t> sink_wdata;
  sc_in<bool> sink_valid;
  sc_out<bool> sink_ready;

  accum_multiply_driver(const sc_module_name &name);
  ~accum_multiply_driver();

  void source();
  void sink();

  SC_HAS_PROCESS(accum_multiply_driver);
};