#pragma once

#include <embedding_lookup.hpp>
#include <feature_interaction.hpp>
#include <mem_controller.hpp>
#include <radsim_config.hpp>
#include <systemc.h>
#include <vector>

class dlrm_top : public sc_module {
private:
  embedding_lookup *embedding_lookup_inst;
  feature_interaction *feature_interaction_inst;
  mem_controller *ext_mem;

public:
  sc_in<bool> rst;
  sc_in<bool> mem_clk;

  sc_in<data_vector<uint64_t>> lookup_indecies_data;
  sc_in<data_vector<unsigned int>> lookup_indecies_target_channels;
  sc_in<data_vector<uint64_t>> lookup_indecies_base_addresses;
  sc_in<bool> lookup_indecies_valid;
  sc_out<bool> lookup_indecies_ready;

  sc_out<data_vector<int16_t>> feature_interaction_odata;
  sc_out<bool> feature_interaction_valid;
  sc_in<bool> feature_interaction_ready;
  sc_out<unsigned int> received_responses;

  dlrm_top(const sc_module_name &name);
  ~dlrm_top();
};