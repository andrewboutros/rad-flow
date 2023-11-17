#pragma once

#include <systemc.h>
#include <vector>
#include <mvm.hpp>
#include <rtl_mvm.hpp>
#include <dispatcher.hpp>
#include <collector.hpp>
#include <weight_loader.hpp>
#include <inst_loader.hpp>
#include <sim_utils.hpp>
#include <design_context.hpp>
#include <radsim_config.hpp>


class mlp_top : public sc_module {
 private:
  std::vector<std::vector<rtl_mvm*>> rtl_matrix_vector_engines;
  std::vector<std::vector<mvm*>> matrix_vector_engines;
  std::vector<unsigned int> num_mvms;
  std::vector<unsigned int> num_mvms_rtl;
  std::vector<unsigned int> num_mvms_total;
  std::vector<dispatcher*> input_dispatchers;
  collector* output_collector;
  weight_loader* wloader;
  inst_loader* iloader;

 public:
  sc_in<bool> rst;
  // Weight loader interfaces
  sc_out<bool> weight_loader_weight_fifo_rdy;
  sc_in<bool> weight_loader_weight_fifo_wen;
  sc_in<data_vector<sc_int<IPRECISION>>> weight_loader_weight_fifo_wdata;
  sc_out<bool> weight_loader_rf_id_fifo_rdy;
  sc_in<bool> weight_loader_rf_id_fifo_wen;
  sc_in<uint8_t> weight_loader_rf_id_fifo_wdata;
  sc_out<bool> weight_loader_rf_addr_fifo_rdy;
  sc_in<bool> weight_loader_rf_addr_fifo_wen;
  sc_in<sc_uint<RF_ADDRW>> weight_loader_rf_addr_fifo_wdata;
  sc_out<bool> weight_loader_layer_id_fifo_rdy;
  sc_in<bool> weight_loader_layer_id_fifo_wen;
  sc_in<uint8_t> weight_loader_layer_id_fifo_wdata;
  sc_out<bool> weight_loader_mvm_id_fifo_rdy;
  sc_in<bool> weight_loader_mvm_id_fifo_wen;
  sc_in<uint8_t> weight_loader_mvm_id_fifo_wdata;
  // Instruction loader interfaces
  sc_out<bool> inst_loader_inst_fifo_rdy;
  sc_in<bool> inst_loader_inst_fifo_wen;
  sc_in<mvm_inst> inst_loader_inst_fifo_wdata;
  sc_out<bool> inst_loader_layer_id_fifo_rdy;
  sc_in<bool> inst_loader_layer_id_fifo_wen;
  sc_in<uint8_t> inst_loader_layer_id_fifo_wdata;
  sc_out<bool> inst_loader_mvm_id_fifo_rdy;
  sc_in<bool> inst_loader_mvm_id_fifo_wen;
  sc_in<uint8_t> inst_loader_mvm_id_fifo_wdata;
  // Dispatchers' interfaces
  sc_vector<sc_out<bool>> dispatcher_fifo_rdy;
  sc_vector<sc_in<bool>> dispatcher_fifo_wen;
  sc_vector<sc_in<data_vector<sc_int<IPRECISION>>>> dispatcher_fifo_wdata;
  // Collector interfaces
  sc_out<bool> collector_fifo_rdy;
  sc_in<bool> collector_fifo_ren;
  sc_out<data_vector<sc_int<IPRECISION>>> collector_fifo_rdata;

  mlp_top(const sc_module_name& name);
  ~mlp_top();
  void prepare_adapters_info();
};