#pragma once

#include <chrono>
#include <mvm.hpp>
#include <mlp_driver.hpp>
#include <mlp_top.hpp>
#include <sim_utils.hpp>
#include <vector>

class mlp_system : public sc_module {
 private:
  sc_vector<sc_signal<bool>> dispatcher_fifo_rdy_signal;
  sc_vector<sc_signal<bool>> dispatcher_fifo_wen_signal;
  sc_vector<sc_signal<data_vector<sc_int<32>>>> dispatcher_fifo_wdata_signal;
  sc_signal<bool> collector_fifo_rdy_signal;
  sc_signal<bool> collector_fifo_ren_signal;
  sc_signal<data_vector<sc_int<32>>> collector_fifo_rdata_signal;

 public:
  sc_signal<bool> rst_sig;
  sc_clock* sysclk;
  mlp_driver* mlp_driver_inst;
  mlp_top* mlp_inst;

  mlp_system(const sc_module_name& name, sc_clock* driver_clk_sig);
  ~mlp_system();
};