#include <producer_consumer_system.hpp>

producer_consumer_system::producer_consumer_system(const sc_module_name &name, sc_clock *driver_clk_sig, RADSimDesignContext* radsim_design) 
    : sc_module(name) {

  // Instantiate design top-level
  dut_inst = new producer_consumer_top("dut", radsim_design);
  dut_inst->rst(rst_sig);
  this->design_dut_inst = dut_inst;
}

producer_consumer_system::~producer_consumer_system() {
  delete dut_inst;
  delete sysclk;
}