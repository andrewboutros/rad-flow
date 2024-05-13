#include <aximm_hello_world_system.hpp>

aximm_hello_world_system::aximm_hello_world_system(const sc_module_name &name,
                                                   sc_clock *driver_clk_sig)
    : sc_module(name) {

  // Instantiate driver
  driver_inst = new aximm_hello_world_driver("driver");
  driver_inst->clk(*driver_clk_sig);
  driver_inst->rst(rst_sig);
  driver_inst->req_addr(req_addr_sig);
  driver_inst->req_type(req_type_sig);
  driver_inst->req_wdata(req_wdata_sig);
  driver_inst->req_valid(req_valid_sig);
  driver_inst->req_ready(req_ready_sig);
  driver_inst->received_responses(received_responses_sig);

  // Instantiate design top-level
  dut_inst = new aximm_hello_world_top("dut");
  dut_inst->rst(rst_sig);
  dut_inst->req_addr(req_addr_sig);
  dut_inst->req_type(req_type_sig);
  dut_inst->req_wdata(req_wdata_sig);
  dut_inst->req_valid(req_valid_sig);
  dut_inst->req_ready(req_ready_sig);
  dut_inst->received_responses(received_responses_sig);
}

aximm_hello_world_system::~aximm_hello_world_system() {
  delete driver_inst;
  delete dut_inst;
  delete sysclk;
}