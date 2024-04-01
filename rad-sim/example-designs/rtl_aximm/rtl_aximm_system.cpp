#include <rtl_aximm_system.hpp>

rtl_aximm_system::rtl_aximm_system(const sc_module_name &name, sc_clock *driver_clk_sig) : sc_module(name) {

  // Instantiate driver
  driver_inst = new rtl_aximm_driver("driver");
  driver_inst->clk(*driver_clk_sig);
  driver_inst->rst(rst_sig);
  driver_inst->start_sig(start_sig);
	driver_inst->start_rdy(start_rdy);
	driver_inst->output_valid(output_valid);
	driver_inst->output_result(output_result);
	driver_inst->output_rdy(output_rdy);

  // Instantiate design top-level
  dut_inst = new rtl_aximm_top("dut");
  dut_inst->rst(rst_sig);
  dut_inst->start_sig(start_sig);
	dut_inst->start_rdy(start_rdy);
	dut_inst->output_valid(output_valid);
	dut_inst->output_result(output_result);
	dut_inst->output_rdy(output_rdy);
}

rtl_aximm_system::~rtl_aximm_system() {
  delete driver_inst;
  delete dut_inst;
  delete sysclk;
}