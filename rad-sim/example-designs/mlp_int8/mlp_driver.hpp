#pragma once

#include <systemc.h>
#include <chrono>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include "mvm.hpp"
#include "sim_utils.hpp"
#include "instructions.hpp"
#include "radsim_config.hpp"

class mlp_driver : public sc_module {
 private:
  int start_cycle, end_cycle;
  std::chrono::steady_clock::time_point start_time, end_time;
  unsigned int num_layers;
  std::vector<unsigned int> num_mvms;
  std::vector<unsigned int> num_mvms_rtl;
  std::vector<unsigned int> num_mvms_total;

  std::vector<std::vector<std::vector<int>>> test_inputs;
  std::vector<std::vector<int>> golden_outputs;

  std::vector<std::vector<int>> weight_data;
  std::vector<unsigned int> weight_rf_id;
  std::vector<unsigned int> weight_rf_addr;
  std::vector<unsigned int> weight_layer_id;
  std::vector<unsigned int> weight_mvm_id;

  std::vector<mvm_inst> inst_data;
  std::vector<unsigned int> inst_layer_id;
  std::vector<unsigned int> inst_mvm_id;

 public:
  sc_in<bool> clk;
  sc_out<bool> rst;
  // Weight loader interfaces
  sc_in<bool> weight_loader_weight_fifo_rdy;
  sc_out<bool> weight_loader_weight_fifo_wen;
  sc_out<data_vector<sc_int<IPRECISION>>> weight_loader_weight_fifo_wdata;
  sc_in<bool> weight_loader_rf_id_fifo_rdy;
  sc_out<bool> weight_loader_rf_id_fifo_wen;
  sc_out<uint8_t> weight_loader_rf_id_fifo_wdata;
  sc_in<bool> weight_loader_rf_addr_fifo_rdy;
  sc_out<bool> weight_loader_rf_addr_fifo_wen;
  sc_out<sc_uint<RF_ADDRW>> weight_loader_rf_addr_fifo_wdata;
  sc_in<bool> weight_loader_layer_id_fifo_rdy;
  sc_out<bool> weight_loader_layer_id_fifo_wen;
  sc_out<uint8_t> weight_loader_layer_id_fifo_wdata;
  sc_in<bool> weight_loader_mvm_id_fifo_rdy;
  sc_out<bool> weight_loader_mvm_id_fifo_wen;
  sc_out<uint8_t> weight_loader_mvm_id_fifo_wdata;
  // Instruction loader interfaces
  sc_in<bool> inst_loader_inst_fifo_rdy;
  sc_out<bool> inst_loader_inst_fifo_wen;
  sc_out<mvm_inst> inst_loader_inst_fifo_wdata;
  sc_in<bool> inst_loader_layer_id_fifo_rdy;
  sc_out<bool> inst_loader_layer_id_fifo_wen;
  sc_out<uint8_t> inst_loader_layer_id_fifo_wdata;
  sc_in<bool> inst_loader_mvm_id_fifo_rdy;
  sc_out<bool> inst_loader_mvm_id_fifo_wen;
  sc_out<uint8_t> inst_loader_mvm_id_fifo_wdata;
  // Dispatchers' interfaces
  sc_vector<sc_in<bool>> dispatcher_fifo_rdy;
  sc_vector<sc_out<bool>> dispatcher_fifo_wen;
  sc_vector<sc_out<data_vector<sc_int<IPRECISION>>>> dispatcher_fifo_wdata;
  // Collector interfaces
  sc_in<bool> collector_fifo_rdy;
  sc_out<bool> collector_fifo_ren;
  sc_in<data_vector<sc_int<IPRECISION>>> collector_fifo_rdata;

  mlp_driver(const sc_module_name& name);
  ~mlp_driver();

  void source();
  void sink();
  void assign();

  SC_HAS_PROCESS(mlp_driver);
};