#pragma once

#include <axis_interface.hpp>
#include <fifo.hpp>
#include <mvm.hpp>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>

class collector : public RADSimModule {
private:
  std::string module_name;

  fifo<int16_t> *data_fifo;
  sc_signal<data_vector<int16_t>> data_fifo_wdata_signal;
  sc_signal<bool> data_fifo_wen_signal, data_fifo_full_signal,
      data_fifo_empty_signal, data_fifo_almost_full_signal,
      data_fifo_almost_empty_signal;

public:
  sc_in<bool> rst;
  sc_out<bool> data_fifo_rdy;
  sc_in<bool> data_fifo_ren;
  sc_out<data_vector<int16_t>> data_fifo_rdata;
  axis_slave_port rx_interface;

  collector(const sc_module_name &name);
  ~collector();

  void Assign();
  SC_HAS_PROCESS(collector);
  void RegisterModuleInfo();
};