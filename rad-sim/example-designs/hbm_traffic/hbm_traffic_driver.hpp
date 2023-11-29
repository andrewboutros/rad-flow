#pragma once

#include <chrono>
#include <embedding_lookup.hpp>
#include <radsim_config.hpp>
#include <sim_utils.hpp>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <traffic_gen.hpp>

// #include <bits/stdc++.h>

class hbm_traffic_driver : public sc_module {
private:
    /*
        std::vector<data_vector<uint64_t>> _lookup_indecies;
        std::vector<std::vector<int16_t>> _feature_interaction_outputs;
    */
    std::vector<std::vector<int16_t>> _mlp_outputs;
    unsigned int _num_feature_interaction_outputs;
    unsigned int _num_mlp_outputs;
    
    unsigned int _start_cycle, _end_cycle;

    /* 
        std::vector<data_vector<unsigned int>> _target_channels;
        std::vector<data_vector<uint64_t>> _base_addresses;
    */

    // unsigned int _num_mem_req_insts; // num of memory request traffic gen instances

    // Traffic Gen    
    std::vector<data_vector<unsigned int>> _target_channels;
    std::vector<data_vector<uint64_t>> _target_addresses;
    std::vector<data_vector<size_t>> _wr_datas; // however many bits are associated with a single memory write tranaction
    std::vector<std::vector<bool>> _wr_ens;
    std::vector<data_vector<uint64_t>> _src_ports;
    std::vector<data_vector<uint64_t>> _dst_ports;

public:
    sc_in<bool> clk;
    sc_out<bool> rst;
    
    /*
        sc_out<data_vector<uint64_t>> lookup_indecies_data;
        sc_out<data_vector<unsigned int>> lookup_indecies_target_channels;
        sc_out<data_vector<uint64_t>> lookup_indecies_base_addresses;
        sc_out<bool> lookup_indecies_valid;
        sc_in<bool> lookup_indecies_ready;
    */

    // Black Box mem request interface
    sc_vector<sc_out<bool>> mem_req_valids;
    sc_vector<sc_in<bool>> mem_req_readys;
    
    sc_vector<sc_out<bool>> wr_ens;
    sc_out<data_vector<unsigned int>> target_channels;
    sc_out<data_vector<uint64_t>> target_addresses;
    sc_out<data_vector<size_t>> wr_datas; // however many bits are associated with a single memory write tranaction
    sc_out<data_vector<uint64_t>> src_ports;
    sc_out<data_vector<uint64_t>> dst_ports;

    // sc_in<unsigned int> received_responses;
    // HBM traffic, sc_vector of size (num_mem_req_insts)
    // sc_vector<sc_out<bool>> mem_req_valid;
    // sc_vector<sc_in<bool>> mem_req_ready;
    // sc_vector<sc_out<bool>> write_en;
    // sc_vector<sc_out<data_vector<uint64_t>>> src_port; // src port of mem req 
    // sc_vector<sc_out<data_vector<uint64_t>>> dst_port; // dst port of mem req
    // sc_vector<sc_out<data_vector<unsigned int>>> target_channel;
    // sc_vector<sc_out<data_vector<uint64_t>>> target_address;

  sc_in<bool> collector_fifo_rdy;
  sc_out<bool> collector_fifo_ren;
  sc_in<data_vector<int16_t>> collector_fifo_rdata;

  hbm_traffic_driver(
    const sc_module_name &name);
  ~hbm_traffic_driver();

  void assign();
  void source();
  void sink();

  SC_HAS_PROCESS(hbm_traffic_driver);
};