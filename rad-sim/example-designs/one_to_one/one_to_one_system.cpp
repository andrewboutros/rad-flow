#include <one_to_one_system.hpp>

one_to_one_system::one_to_one_system(const sc_module_name &name,
                                                   sc_clock *driver_clk_sig)
    : sc_module(name) {

  // Instantiate driver
  driver_inst = new one_to_one_driver("driver");
  driver_inst->clk(*driver_clk_sig);
  driver_inst->rst(rst_sig);
  driver_inst->req_addr(req_addr_sig);
  driver_inst->req_type(req_type_sig);
  driver_inst->req_wdata(req_wdata_sig);
  driver_inst->req_valid(req_valid_sig);
  driver_inst->req_ready(req_ready_sig);
  driver_inst->received_responses(received_responses_sig);

  // Instantiate design top-level
  top_inst = new one_to_one_top("dut");
  top_inst->rst(rst_sig);
  top_inst->req_addr(req_addr_sig);
  top_inst->req_type(req_type_sig);
  top_inst->req_wdata(req_wdata_sig);
  top_inst->req_valid(req_valid_sig);
  top_inst->req_ready(req_ready_sig);
  top_inst->received_responses(received_responses_sig);
}

one_to_one_system::~one_to_one_system() {
  delete driver_inst;
  delete top_inst;
  delete sysclk;
}