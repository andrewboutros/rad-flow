#pragma once

#include <systemc.h>
#include <vector>
#include <string>

#include "radsim_defines.hpp"
#include "axis_interface.hpp"
#include "fifo.hpp"
#include "register_file.hpp"
#include "instructions.hpp"
#include "pipeline.hpp"
#include "datapath.hpp"
#include "config.hpp"

#include <design_context.hpp>

class mvm : public RADSimModule {
 private:
  std::string module_name;
  unsigned int mvm_id;
  unsigned int layer_id;

  fifo<mvm_inst>* instruction_fifo;
  sc_signal<mvm_inst> inst_fifo_odata, inst_fifo_idata;
  sc_signal<bool> inst_fifo_push, inst_fifo_pop, inst_fifo_empty, inst_fifo_full, inst_fifo_almost_full;
  sc_signal<bool> inst_init_fifo_push;
  sc_signal<mvm_inst> inst_init_fifo_idata;

  pipeline<mvm_inst>* inst_rf_pipeline;
  sc_signal<mvm_inst> inst_rf_pipeline_odata;

  pipeline<bool>* inst_valid_rf_pipeline;
  sc_signal<bool> inst_rf_pipeline_valid;

  fifo<data_vector<sc_int<IPRECISION>>>* input_fifo;
  sc_signal<data_vector<sc_int<IPRECISION>>> input_fifo_odata, input_fifo_idata;
  sc_signal<bool> input_fifo_push, input_fifo_pop, input_fifo_empty, input_fifo_full, input_fifo_almost_full;

  fifo<data_vector<sc_int<IPRECISION>>>* reduction_fifo;
  sc_signal<data_vector<sc_int<IPRECISION>>> reduction_fifo_odata, reduction_fifo_idata;
  sc_signal<bool> reduction_fifo_push, reduction_fifo_pop, reduction_fifo_empty, reduction_fifo_full, 
    reduction_fifo_almost_full; 

  pipeline<data_vector<sc_int<IPRECISION>>>* input_pipeline;
  pipeline<data_vector<sc_int<IPRECISION>>>* reduction_pipeline;
  sc_signal<data_vector<sc_int<IPRECISION>>> input_pipeline_odata, reduction_pipeline_odata;

  std::vector<register_file<sc_int<IPRECISION>>*> rf;
  sc_vector<sc_signal<data_vector<sc_int<IPRECISION>>>> rf_rdata;
  sc_signal<data_vector<sc_int<IPRECISION>>> rf_wdata;
  sc_vector<sc_signal<bool>> rf_wen;
  sc_signal<uint32_t> rf_raddr, rf_waddr;
  sc_signal<bool> rf_clk_en;

  std::vector<datapath*> datapath_inst;
  sc_vector<sc_signal<sc_int<IPRECISION>>> datapath_reduction_operand;
  sc_vector<sc_signal<bool>> datapath_ovalid;
  sc_vector<sc_signal<sc_int<OPRECISION>>> datapath_results;
  sc_signal<uint32_t> datapath_inst_accum;
  sc_signal<bool> datapath_inst_accum_en, datapath_inst_reduce, datapath_inst_release;

  pipeline<mvm_inst>* inst_datapath_pipeline;
  sc_signal<mvm_inst> inst_datapath_pipeline_odata;

  fifo<data_vector<sc_int<IPRECISION>>>* output_data_fifo;
  sc_signal<data_vector<sc_int<IPRECISION>>> output_data_fifo_odata, output_data_fifo_idata;
  sc_signal<bool> output_data_fifo_push, output_data_fifo_pop, output_data_fifo_empty, output_data_fifo_full, 
    output_data_fifo_almost_full;

  // wires
  sc_signal<uint32_t> inst_rf_raddr;
  sc_signal<uint32_t> tuser_rf_addr, tuser_op;
  sc_vector<sc_signal<bool>> tuser_rf_en;

 public:
  sc_in<bool> rst;
  axis_slave_port rx_interface;
  axis_master_port tx_interface;

  mvm(const sc_module_name& name, unsigned int id_mvm, unsigned int id_layer);
  ~mvm();

  void Assign();
  void Tick();
  SC_HAS_PROCESS(mvm);
  void RegisterModuleInfo();
};