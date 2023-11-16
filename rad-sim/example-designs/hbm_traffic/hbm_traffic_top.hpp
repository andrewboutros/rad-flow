#pragma once

#include <collector.hpp>
#include <custom_feature_interaction.hpp>
#include <embedding_lookup.hpp>
#include <feature_interaction.hpp>
#include <mem_controller.hpp>
#include <mvm.hpp>
#include <black_box.hpp>
#include <radsim_config.hpp>
#include <systemc.h>
#include <vector>

class hbm_traffic_top : public sc_module {
private:
  embedding_lookup *embedding_lookup_inst;
  custom_feature_interaction *feature_interaction_inst;
  std::vector<black_box *> black_boxes;
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

    // Black Box mem request interface
    sc_in<data_vector<bool>> mem_req_valids;
    sc_out<data_vector<bool>> mem_req_readys;
    
    sc_in<bool> write_en;
    sc_in<data_vector<unsigned int>> target_channels;
    sc_in<data_vector<uint64_t>> target_addresses;
    sc_in<data_vector<uint16_t>> wr_datas; //v
    sc_in<data_vector<bool>> wr_ens; //v
    sc_in<data_vector<uint64_t>> src_ports;
    sc_in<data_vector<uint64_t>> dst_ports;

  
  

  // sc_out<unsigned int> received_responses;

  sc_out<bool> collector_fifo_rdy;
  sc_in<bool> collector_fifo_ren;
  sc_out<data_vector<int16_t>> collector_fifo_rdata;

  hbm_traffic_top(const sc_module_name &name);
  ~hbm_traffic_top();
};