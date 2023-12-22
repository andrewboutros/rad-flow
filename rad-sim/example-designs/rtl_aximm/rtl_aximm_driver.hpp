#pragma once

#include <chrono>
#include <radsim_config.hpp>
#include <design_context.hpp>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>
#include <vector>

class rtl_aximm_driver : public sc_module {
public:
  sc_in<bool> clk;
  sc_out<bool> rst;
  // Client Interface
  sc_out<bool> start_sig;
	sc_in<bool> start_rdy;
	sc_in<bool> output_valid;
	sc_in<bool> output_result;
	sc_out<bool> output_rdy;

  rtl_aximm_driver(const sc_module_name &name);
  ~rtl_aximm_driver();

  void source();
  void sink();

  SC_HAS_PROCESS(rtl_aximm_driver);
};