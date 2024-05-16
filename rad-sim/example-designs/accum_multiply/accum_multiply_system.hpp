#pragma once

#include <accum_multiply_driver.hpp>
#include <accum_multiply_top.hpp>
#include <chrono>
#include <vector>

class aximm_hello_world_system : public sc_module {
private:
  sc_signal<int16_t> input;
  sc_signal<bool> input_valid;
  sc_signal<bool> input_ready;
  sc_signal<int16_t> output;
  sc_signal<bool> output_ready;
  sc_signal<bool> output_valid;

public:
  sc_signal<bool> rst_sig;
  sc_clock *sysclk;
  accum_multiply_driver *driver_inst;
  accum_multiply_top *dut_inst;

  aximm_hello_world_system(const sc_module_name &name,
                           sc_clock *driver_clk_sig);
  ~aximm_hello_world_system();
};