#pragma once

#include <systemc.h>
#include <vector>
#include <string>

#include "radsim_defines.hpp"
#include "axis_interface.hpp"
#include "fifo.hpp"
#include "mvm.hpp"
#include "radsim_module.hpp"

class dispatcher : public RADSimModule {
 private:
  std::string module_name;
  unsigned int dispatcher_id;

  fifo<sc_int<32>>* data_fifo;
  sc_signal<data_vector<sc_int<32>>> data_fifo_rdata_signal;
  sc_signal<bool> data_fifo_ren_signal, data_fifo_full_signal, data_fifo_empty_signal, 
    data_fifo_almost_full_signal, data_fifo_almost_empty_signal;

 public:
  sc_in<bool> rst;
  sc_out<bool> data_fifo_rdy;
  sc_in<bool> data_fifo_wen;
  sc_in<data_vector<sc_int<32>>> data_fifo_wdata;
  axis_master_port tx_interface;

  dispatcher(const sc_module_name& name, unsigned int id);
  ~dispatcher();

  void Assign();
  SC_HAS_PROCESS(dispatcher);
  void RegisterModuleInfo();
};