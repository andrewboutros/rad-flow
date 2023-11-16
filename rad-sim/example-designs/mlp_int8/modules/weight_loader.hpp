#pragma once

#include <systemc.h>
#include <vector>
#include <string>

#include "radsim_defines.hpp"
#include "axis_interface.hpp"
#include "fifo.hpp"
#include "radsim_module.hpp"
#include "config.hpp"
#include "design_context.hpp"

class weight_loader : public RADSimModule {
 private:
  std::string module_name;

  fifo<data_vector<sc_int<IPRECISION>>>* weight_fifo;
  sc_signal<data_vector<sc_int<IPRECISION>>> weight_fifo_odata;
  sc_signal<bool> weight_fifo_pop, weight_fifo_full, weight_fifo_empty, weight_fifo_almost_full;
  
  fifo<uint8_t>* rf_id_fifo;
  sc_signal<uint8_t> rf_id_fifo_odata;
  sc_signal<bool> rf_id_fifo_pop, rf_id_fifo_full, rf_id_fifo_empty, rf_id_fifo_almost_full;
  
  fifo<sc_uint<RF_ADDRW>>* rf_addr_fifo;
  sc_signal<sc_uint<RF_ADDRW>> rf_addr_fifo_odata;
  sc_signal<bool> rf_addr_fifo_pop, rf_addr_fifo_full, rf_addr_fifo_empty, rf_addr_fifo_almost_full;

  fifo<uint8_t>* layer_id_fifo;
  sc_signal<uint8_t> layer_id_fifo_odata;
  sc_signal<bool> layer_id_fifo_pop, layer_id_fifo_full, layer_id_fifo_empty, layer_id_fifo_almost_full;

  fifo<uint8_t>* mvm_id_fifo;
  sc_signal<uint8_t> mvm_id_fifo_odata;
  sc_signal<bool> mvm_id_fifo_pop, mvm_id_fifo_full, mvm_id_fifo_empty, mvm_id_fifo_almost_full;

 public:
  sc_in<bool> rst;
  sc_out<bool> weight_fifo_rdy;
  sc_in<bool> weight_fifo_wen;
  sc_in<data_vector<sc_int<IPRECISION>>> weight_fifo_wdata;
  sc_out<bool> rf_id_fifo_rdy;
  sc_in<bool> rf_id_fifo_wen;
  sc_in<uint8_t> rf_id_fifo_wdata;
  sc_out<bool> rf_addr_fifo_rdy;
  sc_in<bool> rf_addr_fifo_wen;
  sc_in<sc_uint<RF_ADDRW>> rf_addr_fifo_wdata;
  sc_out<bool> layer_id_fifo_rdy;
  sc_in<bool> layer_id_fifo_wen;
  sc_in<uint8_t> layer_id_fifo_wdata;
  sc_out<bool> mvm_id_fifo_rdy;
  sc_in<bool> mvm_id_fifo_wen;
  sc_in<uint8_t> mvm_id_fifo_wdata;
  axis_master_port tx_interface;

  weight_loader(const sc_module_name& name);
  ~weight_loader();

  void Assign();
  SC_HAS_PROCESS(weight_loader);
  void RegisterModuleInfo();
};