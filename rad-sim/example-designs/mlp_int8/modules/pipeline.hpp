#pragma once

#include <systemc.h>
#include <sim_utils.hpp>
#include <vector>
#include <radsim_telemetry.hpp>
#include "config.hpp"
#include "instructions.hpp"

template <class dtype>
class pipeline : public sc_module {
 private:
  unsigned int pipeline_depth;
  sc_vector<sc_signal<dtype>> pipeline_reg;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_in<dtype> idata;
  sc_out<dtype> odata;

  pipeline(const sc_module_name& name, unsigned int depth);
  ~pipeline();

  void Tick();
  void Assign();
  SC_HAS_PROCESS(pipeline);
};