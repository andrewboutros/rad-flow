#pragma once

#include <chrono>
#include <radsim_config.hpp>
#include <client_mult.hpp>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>
#include <queue>

class mult_driver : public sc_module {
private:
  int start_cycle, end_cycle;
  std::queue<int> numbers_to_send;
  int actual_product;
  RADSimDesignContext* radsim_design; //store ptr passed into constructor for use in source() and sink()
  
public:
  sc_in<bool> clk;
  sc_out<bool> rst;
  sc_out<sc_bv<DATAW>> client_tdata;
  sc_out<bool> client_tlast;
  sc_out<bool> client_valid;
  sc_in<bool> client_ready;
  sc_in<sc_bv<DATAW>> response;
  sc_in<bool> response_valid;
  sc_in<bool> mult_inter_rad_recvd;

  mult_driver(const sc_module_name &name, RADSimDesignContext* radsim_design_);
  ~mult_driver();

  void source();
  void sink();

  SC_HAS_PROCESS(mult_driver);
};