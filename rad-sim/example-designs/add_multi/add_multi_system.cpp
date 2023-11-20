#include <add_multi_system.hpp>

add_multi_system::add_multi_system(const sc_module_name &name, sc_clock *driver_clk_sig)
    : sc_module(name) {

  // Instantiate driver
  driver_inst = new add_driver("driver");
  driver_inst->clk(*driver_clk_sig);
  driver_inst->rst(rst_sig);
  driver_inst->client_tdata(client_tdata_sig);
  driver_inst->client_tlast(client_tlast_sig);
  driver_inst->client_valid(client_valid_sig);
  driver_inst->client_ready(client_ready_sig);
  driver_inst->client_tsel_data(client_tsel_data_sig);
  driver_inst->client_tsel_valid(client_tsel_valid_sig);
  driver_inst->response(response_sig);
  driver_inst->response_valid(response_valid_sig);
  driver_inst->response2(response_sig2);
  driver_inst->response_valid2(response_valid_sig2);

  // Instantiate design top-level
  dut_inst = new add_top("dut");
  dut_inst->rst(rst_sig);
  dut_inst->client_tdata(client_tdata_sig);
  dut_inst->client_tlast(client_tlast_sig);
  dut_inst->client_valid(client_valid_sig);
  dut_inst->client_ready(client_ready_sig);
  dut_inst->client_tsel_data(client_tsel_data_sig);
  dut_inst->client_tsel_valid(client_tsel_valid_sig);
  dut_inst->response(response_sig);
  dut_inst->response_valid(response_valid_sig);
  //for adder_inst2
  dut_inst->response2(response_sig2);
  dut_inst->response_valid2(response_valid_sig2);
}

add_multi_system::~add_multi_system() {
  delete driver_inst;
  delete dut_inst;
  delete sysclk;
}
