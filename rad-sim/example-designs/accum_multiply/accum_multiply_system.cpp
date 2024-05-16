#include <accum_multiply_system.hpp>

accum_multiply_system::accum_multiply_system(const sc_module_name &name,
                                                   sc_clock *driver_clk_sig)
    : sc_module(name) {

  // Instantiate driver
  driver_inst = new accum_multiply_driver("driver");
  driver_inst->clk(*driver_clk_sig);
  driver_inst->rst(rst_sig);
  driver_inst->source_wdata(input);
  driver_inst->source_valid(input_valid);
  driver_inst->source_ready(input_ready);
  driver_inst->sink_rdata(output);
  driver_inst->sink_valid(output_ready);
  driver_inst->sink_ready(output_valid);

  // Instantiate design top-level
  dut_inst = new accum_multiply_top("dut");
  dut_inst->rst(rst_sig);
  dut_inst->input(input);
  dut_inst->input_valid(input_valid);
  dut_inst->input_ready(input_ready);
  dut_inst->output(output);
  dut_inst->output_ready(output_ready);
  dut_inst->output_valid(output_valid);
}

accum_multiply_system::~accum_multiply_system() {
  delete driver_inst;
  delete dut_inst;
  delete sysclk;
}