#pragma once

#include <collector.hpp>
#include <custom_feature_interaction.hpp>
#include <embedding_lookup.hpp>
#include <feature_interaction.hpp>
#include <mem_controller.hpp>
#include <mvm.hpp>
#include <radsim_config.hpp>
#include <systemc.h>
#include <vector>

class dlrm_top : public sc_module {
private:
  embedding_lookup *embedding_lookup_inst;
  custom_feature_interaction *feature_interaction_inst;
  std::vector<std::vector<mvm *>> mvms;
  collector *output_collector;
  std::vector<mem_controller *> ext_mem;

  std::vector<axis_signal> axis_sig;
  std::vector<sc_clock *> mem_clks;

public:
  sc_in<bool> rst;

  sc_in<data_vector<uint64_t>> lookup_indecies_data;
  sc_in<data_vector<unsigned int>> lookup_indecies_target_channels;
  sc_in<data_vector<uint64_t>> lookup_indecies_base_addresses;
  sc_in<bool> lookup_indecies_valid;
  sc_out<bool> lookup_indecies_ready;

  sc_out<unsigned int> received_responses;

  sc_out<bool> collector_fifo_rdy;
  sc_in<bool> collector_fifo_ren;
  sc_out<data_vector<int16_t>> collector_fifo_rdata;

  dlrm_top(const sc_module_name &name);
  ~dlrm_top();
};