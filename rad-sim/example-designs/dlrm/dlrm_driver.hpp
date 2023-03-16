#pragma once

#include <chrono>
#include <embedding_lookup.hpp>
#include <radsim_config.hpp>
#include <sim_utils.hpp>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>
#include <vector>

class dlrm_driver : public sc_module {
private:
  std::vector<data_vector<uint64_t>> _lookup_indecies;
  std::vector<data_vector<unsigned int>> _target_channels;
  std::vector<data_vector<uint64_t>> _base_addresses;
  std::vector<std::vector<int16_t>> _feature_interaction_outputs;
  unsigned int _num_outputs;
  unsigned int _start_cycle, _end_cycle;

public:
  sc_in<bool> clk;
  sc_out<bool> rst;
  sc_out<data_vector<uint64_t>> lookup_indecies_data;
  sc_out<data_vector<unsigned int>> lookup_indecies_target_channels;
  sc_out<data_vector<uint64_t>> lookup_indecies_base_addresses;
  sc_out<bool> lookup_indecies_valid;
  sc_in<bool> lookup_indecies_ready;

  sc_in<data_vector<int16_t>> feature_interaction_odata;
  sc_in<bool> feature_interaction_valid;
  sc_out<bool> feature_interaction_ready;
  sc_in<unsigned int> received_responses;

  dlrm_driver(const sc_module_name &name);
  ~dlrm_driver();

  void source();
  void sink();

  SC_HAS_PROCESS(dlrm_driver);
};