#pragma once

#include <one_to_one_driver.hpp>
#include <one_to_one_top.hpp>
#include <chrono>
#include <vector>

class one_to_one_system : public sc_module {
private:
  sc_signal<data_vector<uint64_t>> lookup_indecies_data_sig;
  sc_signal<data_vector<unsigned int>> lookup_indecies_target_channels_sig;
  sc_signal<data_vector<uint64_t>> lookup_indecies_base_addresses_sig;
  sc_signal<bool> lookup_indecies_valid_sig;
  sc_signal<bool> lookup_indecies_ready_sig;

  sc_signal<unsigned int> received_responses_sig;

  sc_signal<bool> collector_fifo_rdy_sig;
  sc_signal<bool> collector_fifo_ren_sig;
  sc_signal<data_vector<int16_t>> collector_fifo_rdata_sig;

public:
  sc_signal<bool> rst_sig;
  one_to_one_driver *driver_inst;
  one_to_one_top *dut_inst;

  dlrm_system(const sc_module_name &name, sc_clock *driver_clk_sig);
  ~dlrm_system();
};