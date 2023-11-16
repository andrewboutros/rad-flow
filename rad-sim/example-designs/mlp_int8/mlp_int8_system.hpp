#pragma once

#include <chrono>
#include <mvm.hpp>
#include <mlp_driver.hpp>
#include <mlp_top.hpp>
#include <sim_utils.hpp>
#include <vector>

class mlp_int8_system : public sc_module {
 private:
  sc_signal<bool> weight_loader_weight_fifo_rdy_signal;
  sc_signal<bool> weight_loader_weight_fifo_wen_signal;
  sc_signal<data_vector<sc_int<IPRECISION>>> weight_loader_weight_fifo_wdata_signal;
  sc_signal<bool> weight_loader_rf_id_fifo_rdy_signal;
  sc_signal<bool> weight_loader_rf_id_fifo_wen_signal;
  sc_signal<uint8_t> weight_loader_rf_id_fifo_wdata_signal;
  sc_signal<bool> weight_loader_rf_addr_fifo_rdy_signal;
  sc_signal<bool> weight_loader_rf_addr_fifo_wen_signal;
  sc_signal<sc_uint<RF_ADDRW>> weight_loader_rf_addr_fifo_wdata_signal;
  sc_signal<bool> weight_loader_layer_id_fifo_rdy_signal;
  sc_signal<bool> weight_loader_layer_id_fifo_wen_signal;
  sc_signal<uint8_t> weight_loader_layer_id_fifo_wdata_signal;
  sc_signal<bool> weight_loader_mvm_id_fifo_rdy_signal;
  sc_signal<bool> weight_loader_mvm_id_fifo_wen_signal;
  sc_signal<uint8_t> weight_loader_mvm_id_fifo_wdata_signal;

  sc_signal<bool> inst_loader_inst_fifo_rdy_signal;
  sc_signal<bool> inst_loader_inst_fifo_wen_signal;
  sc_signal<mvm_inst> inst_loader_inst_fifo_wdata_signal;
  sc_signal<bool> inst_loader_layer_id_fifo_rdy_signal;
  sc_signal<bool> inst_loader_layer_id_fifo_wen_signal;
  sc_signal<uint8_t> inst_loader_layer_id_fifo_wdata_signal;
  sc_signal<bool> inst_loader_mvm_id_fifo_rdy_signal;
  sc_signal<bool> inst_loader_mvm_id_fifo_wen_signal;
  sc_signal<uint8_t> inst_loader_mvm_id_fifo_wdata_signal;

  sc_vector<sc_signal<bool>> dispatcher_fifo_rdy_signal;
  sc_vector<sc_signal<bool>> dispatcher_fifo_wen_signal;
  sc_vector<sc_signal<data_vector<sc_int<IPRECISION>>>> dispatcher_fifo_wdata_signal;
  sc_signal<bool> collector_fifo_rdy_signal;
  sc_signal<bool> collector_fifo_ren_signal;
  sc_signal<data_vector<sc_int<IPRECISION>>> collector_fifo_rdata_signal;

 public:
  sc_signal<bool> rst_sig;
  sc_clock* sysclk;
  mlp_driver* mlp_driver_inst;
  mlp_top* mlp_inst;

  mlp_int8_system(const sc_module_name& name, sc_clock* driver_clk_sig);
  ~mlp_int8_system();
};