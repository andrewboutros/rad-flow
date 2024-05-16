#pragma once

#include <radsim_config.hpp>
#include <adder.hpp>
#include <multiplier.hpp>
#include <systemc.h>
#include <vector>

class accum_multiply_top : public sc_module {
private:
  adder *adder_inst;
  multiplier *multiplier_inst;

public:
  sc_in<bool> rst;
  // Adder's interface
  sc_in<int16_t> input;
  sc_in<bool> input_valid;
  sc_out<bool> input_ready;
  // Multiplier's interface
  sc_out<int16_t> output;
  sc_in<bool> output_ready;
  sc_out<bool> output_valid;

  accum_multiply_top(const sc_module_name &name);
  ~accum_multiply_top();
};