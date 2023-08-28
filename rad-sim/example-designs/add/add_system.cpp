#include <add_system.hpp>

add_system::add_system(const sc_module_name &name, sc_clock *driver_clk_sig)
    : sc_module(name) {

  // Instantiate driver
  driver_inst = new add_driver("driver");
  driver_inst->clk(*driver_clk_sig);
  driver_inst->rst(rst_sig);
  driver_inst->client_tdata(client_tdata_sig);
  driver_inst->client_tlast(client_tlast_sig);
  driver_inst->client_valid(client_valid_sig);
  driver_inst->client_ready(client_ready_sig);

  // Instantiate design top-level
  dut_inst = new add_top("dut");
  dut_inst->rst(rst_sig);
  dut_inst->client_tdata(client_tdata_sig);
  dut_inst->client_tlast(client_tlast_sig);
  dut_inst->client_valid(client_valid_sig);
  dut_inst->client_ready(client_ready_sig);
}

add_system::~add_system() {
  delete driver_inst;
  delete dut_inst;
  delete sysclk;
}