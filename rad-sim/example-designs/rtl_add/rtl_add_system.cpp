#include <rtl_add_system.hpp>

rtl_add_system::rtl_add_system(const sc_module_name &name, sc_clock *driver_clk_sig)
    : sc_module(name) {

  // Instantiate driver
  driver_inst = new rtl_add_driver("driver");
  driver_inst->clk(*driver_clk_sig);
  driver_inst->rst(rst_sig);
  driver_inst->client_tdata(client_tdata_sig);
  driver_inst->client_tlast(client_tlast_sig);
  driver_inst->client_valid(client_valid_sig);
  driver_inst->client_ready(client_ready_sig);
  driver_inst->response(response_sig);
  driver_inst->response_valid(response_valid_sig);

  // Instantiate design top-level
  dut_inst = new rtl_add_top("dut");
  dut_inst->rst(rst_sig);
  dut_inst->client_tdata(client_tdata_sig);
  dut_inst->client_tlast(client_tlast_sig);
  dut_inst->client_valid(client_valid_sig);
  dut_inst->client_ready(client_ready_sig);
  dut_inst->response(response_sig);
  dut_inst->response_valid(response_valid_sig);
}

rtl_add_system::~rtl_add_system() {
  delete driver_inst;
  delete dut_inst;
  delete sysclk;
}