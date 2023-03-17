#include <dlrm_system.hpp>

dlrm_system::dlrm_system(const sc_module_name &name, sc_clock *driver_clk_sig,
                         sc_clock *mem_clk_sig)
    : sc_module(name) {

  // Instantiate driver
  driver_inst = new dlrm_driver("driver");
  driver_inst->clk(*driver_clk_sig);
  driver_inst->rst(rst_sig);
  driver_inst->lookup_indecies_data(lookup_indecies_data_sig);
  driver_inst->lookup_indecies_target_channels(
      lookup_indecies_target_channels_sig);
  driver_inst->lookup_indecies_base_addresses(
      lookup_indecies_base_addresses_sig);
  driver_inst->lookup_indecies_valid(lookup_indecies_valid_sig);
  driver_inst->lookup_indecies_ready(lookup_indecies_ready_sig);
  driver_inst->received_responses(received_responses_sig);
  driver_inst->collector_fifo_rdy(collector_fifo_rdy_sig);
  driver_inst->collector_fifo_ren(collector_fifo_ren_sig);
  driver_inst->collector_fifo_rdata(collector_fifo_rdata_sig);

  // Instantiate design top-level
  dut_inst = new dlrm_top("dut");
  dut_inst->mem_clk(*mem_clk_sig);
  dut_inst->rst(rst_sig);
  dut_inst->lookup_indecies_data(lookup_indecies_data_sig);
  dut_inst->lookup_indecies_target_channels(
      lookup_indecies_target_channels_sig);
  dut_inst->lookup_indecies_base_addresses(lookup_indecies_base_addresses_sig);
  dut_inst->lookup_indecies_valid(lookup_indecies_valid_sig);
  dut_inst->lookup_indecies_ready(lookup_indecies_ready_sig);
  dut_inst->received_responses(received_responses_sig);
  dut_inst->collector_fifo_rdy(collector_fifo_rdy_sig);
  dut_inst->collector_fifo_ren(collector_fifo_ren_sig);
  dut_inst->collector_fifo_rdata(collector_fifo_rdata_sig);
}

dlrm_system::~dlrm_system() {
  delete driver_inst;
  delete dut_inst;
}