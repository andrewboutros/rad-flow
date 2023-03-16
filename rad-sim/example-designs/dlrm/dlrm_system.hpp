#pragma once

#include <chrono>
#include <dlrm_driver.hpp>
#include <dlrm_top.hpp>
#include <vector>

class dlrm_system : public sc_module {
private:
  sc_signal<data_vector<uint64_t>> lookup_indecies_data_sig;
  sc_signal<data_vector<unsigned int>> lookup_indecies_target_channels_sig;
  sc_signal<data_vector<uint64_t>> lookup_indecies_base_addresses_sig;
  sc_signal<bool> lookup_indecies_valid_sig;
  sc_signal<bool> lookup_indecies_ready_sig;

  sc_signal<data_vector<int16_t>> feature_interaction_odata_sig;
  sc_signal<bool> feature_interaction_valid_sig;
  sc_signal<bool> feature_interaction_ready_sig;
  sc_signal<unsigned int> received_responses_sig;

public:
  sc_signal<bool> rst_sig;
  dlrm_driver *driver_inst;
  dlrm_top *dut_inst;

  dlrm_system(const sc_module_name &name, sc_clock *driver_clk_sig,
              sc_clock *mem_clk_sig);
  ~dlrm_system();
};