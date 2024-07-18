#pragma once

#include <systemc.h>
#include <vector>
#include <mvm.hpp>
#include <dispatcher.hpp>
#include <collector.hpp>
#include <sim_utils.hpp>
#include <design_context.hpp>
#include <radsim_config.hpp>
#include <design_top.hpp>

class mlp_top : public design_top {
 private:
  std::vector<std::vector<mvm*>> matrix_vector_engines;
  std::vector<dispatcher*> input_dispatchers;
  collector* output_collector;
  RADSimDesignContext* radsim_design; //AKB ADDED

 public:
  sc_in<bool> rst;
  // Dispatchers' interfaces
  sc_vector<sc_out<bool>> dispatcher_fifo_rdy;
  sc_vector<sc_in<bool>> dispatcher_fifo_wen;
  sc_vector<sc_in<data_vector<sc_int<32>>>> dispatcher_fifo_wdata;
  // Collector interfaces
  sc_out<bool> collector_fifo_rdy;
  sc_in<bool> collector_fifo_ren;
  sc_out<data_vector<sc_int<32>>> collector_fifo_rdata;

  mlp_top(const sc_module_name& name, RADSimDesignContext* radsim_design);
  ~mlp_top();
  void prepare_adapters_info();
};