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
  unsigned int num_layers;
  std::vector<unsigned int> num_mvms;
  std::vector<std::vector<std::vector<int>>> test_inputs;
  std::vector<std::vector<int>> golden_outputs;
  RADSimDesignContext* radsim_design; //AKB ADDED

 public:
  sc_in<bool> clk;
  sc_out<bool> rst;
  // Dispatchers' interfaces
  sc_vector<sc_in<bool>> dispatcher_fifo_rdy;
  sc_vector<sc_out<bool>> dispatcher_fifo_wen;
  sc_vector<sc_out<data_vector<sc_int<32>>>> dispatcher_fifo_wdata;
  // Collector interfaces
  sc_in<bool> collector_fifo_rdy;
  sc_out<bool> collector_fifo_ren;
  sc_in<data_vector<sc_int<32>>> collector_fifo_rdata;

  mlp_driver(const sc_module_name& name, RADSimDesignContext* radsim_design_);
  ~mlp_driver();

  void source();
  void sink();
  void assign();

  SC_HAS_PROCESS(mlp_driver);
};