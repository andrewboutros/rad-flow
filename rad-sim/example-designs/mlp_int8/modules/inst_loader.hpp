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
#include "instructions.hpp"

class inst_loader : public RADSimModule {
 private:
  std::string module_name;

  fifo<mvm_inst>* inst_fifo;
  sc_signal<mvm_inst> inst_fifo_odata;
  sc_signal<bool> inst_fifo_pop, inst_fifo_full, inst_fifo_empty, inst_fifo_almost_full;
  
  fifo<uint8_t>* layer_id_fifo;
  sc_signal<uint8_t> layer_id_fifo_odata;
  sc_signal<bool> layer_id_fifo_pop, layer_id_fifo_full, layer_id_fifo_empty, layer_id_fifo_almost_full;

  fifo<uint8_t>* mvm_id_fifo;
  sc_signal<uint8_t> mvm_id_fifo_odata;
  sc_signal<bool> mvm_id_fifo_pop, mvm_id_fifo_full, mvm_id_fifo_empty, mvm_id_fifo_almost_full;

 public:
  RADSimDesignContext* radsim_design;
  sc_in<bool> rst;
  sc_out<bool> inst_fifo_rdy;
  sc_in<bool> inst_fifo_wen;
  sc_in<mvm_inst> inst_fifo_wdata;
  sc_out<bool> layer_id_fifo_rdy;
  sc_in<bool> layer_id_fifo_wen;
  sc_in<uint8_t> layer_id_fifo_wdata;
  sc_out<bool> mvm_id_fifo_rdy;
  sc_in<bool> mvm_id_fifo_wen;
  sc_in<uint8_t> mvm_id_fifo_wdata;
  axis_master_port tx_interface;

  inst_loader(const sc_module_name& name, RADSimDesignContext* radsim_design);
  ~inst_loader();

  void Assign();
  SC_HAS_PROCESS(inst_loader);
  void RegisterModuleInfo();
};