#pragma once

#include <chrono>
#include <hbm_traffic_driver.hpp>
#include <hbm_traffic_top.hpp>
#include <vector>

// System contains all actual signals we are connecting from driver to top device

class hbm_traffic_system : public sc_module {
private:
  sc_signal<data_vector<uint64_t>> lookup_indecies_data_sig;
  sc_signal<data_vector<unsigned int>> lookup_indecies_target_channels_sig;
  sc_signal<data_vector<uint64_t>> lookup_indecies_base_addresses_sig;
  sc_signal<bool> lookup_indecies_valid_sig;
  sc_signal<bool> lookup_indecies_ready_sig;

  // sc_signal<unsigned int> received_responses_sig;
  // using data_vector to create a vector of signals without creating ports
  sc_signal<data_vector<bool>> mem_req_readys_sig;
  sc_signal<data_vector<bool>> mem_req_valids_sig;
  
  // 
  sc_signal<data_vector<size_t>> wr_datas_sig; // TODO allow for variable bitwidth
  sc_signal<data_vector<unsigned int>> target_channels_sig;
  sc_signal<data_vector<uint64_t>> target_addresses_sig;
  sc_signal<data_vector<uint64_t>> src_ports_sig;
  sc_signal<data_vector<uint64_t>> dst_ports_sig;
  sc_signal<data_vector<bool>> wr_ens_sig;


  sc_signal<bool> collector_fifo_rdy_sig;
  sc_signal<bool> collector_fifo_ren_sig;
  sc_signal<data_vector<int16_t>> collector_fifo_rdata_sig;

public:
  sc_signal<bool> rst_sig;
  hbm_traffic_driver *driver_inst;
  hbm_traffic_top *dut_inst;

  hbm_traffic_system(const sc_module_name &name, sc_clock *driver_clk_sig);
  ~hbm_traffic_system();
};