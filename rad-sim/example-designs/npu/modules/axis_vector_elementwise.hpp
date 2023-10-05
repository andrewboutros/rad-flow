#pragma once

#include <systemc.h>

#include <axis_fifo_adapters.hpp>
#include <axis_interface.hpp>
#include <decoders.hpp>
#include <evrf.hpp>
#include <loader.hpp>
#include <mfu.hpp>
#include <radsim_module.hpp>

class axis_vector_elementwise : public RADSimModule {
 private:
  std::string _name;
  unsigned int _thread_id;

  // Sub-modules
  std::vector<evrf_decoder*> evrf_inst_decoders;
  std::vector<mfu_decoder*> mfu0_inst_decoders;
  std::vector<mfu_decoder*> mfu1_inst_decoders;
  ld_decoder* ld_inst_decoder;

  std::vector<evrf*> evrf_modules;
  std::vector<mfu*> mfu0_modules;
  std::vector<mfu*> mfu1_modules;
  loader* ld_module;

  // Slave AXI-streaming interfaces for instructions and inputs, and master AXI-streaming interfaces for outputs
  std::vector<axis_slave_fifo_adapter<evrf_mop, sc_bv<EVRF_MOP_BITWIDTH>>*> evrf_inst_axis_interfaces;
  std::vector<axis_slave_fifo_adapter<mfu_mop, sc_bv<MFU_MOP_BITWIDTH>>*> mfu0_inst_axis_interfaces;
  std::vector<axis_slave_fifo_adapter<mfu_mop, sc_bv<MFU_MOP_BITWIDTH>>*> mfu1_inst_axis_interfaces;
  axis_slave_fifo_adapter<ld_mop, sc_bv<LD_MOP_BITWIDTH>>* ld_inst_axis_interface;

  std::vector<axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>*>
      evrf_ififo_axis_interfaces;
  axis_master_fifo_adapter<data_vector<tb_input_precision>, sc_bv<WIDE_WRITEBACK_BV_WIDTH>>* ld_wb0_axis_interface;

  // Signals for connecting between different components of the wrapper
  sc_vector<sc_vector<sc_signal<bool>>> evrf_ififo_rdy_signal, evrf_ififo_ren_signal;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> evrf_ififo_rdata_signal;
  sc_vector<sc_vector<sc_signal<bool>>> mfu0_ififo_rdy_signal, mfu0_ififo_ren_signal;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> mfu0_ififo_rdata_signal;
  sc_vector<sc_vector<sc_signal<bool>>> mfu1_ififo_rdy_signal, mfu1_ififo_ren_signal;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> mfu1_ififo_rdata_signal;
  sc_vector<sc_vector<sc_signal<bool>>> ld_ififo_rdy_signal, ld_ififo_ren_signal;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> ld_ififo_rdata_signal;
  sc_vector<sc_signal<bool>> ld_wb0_rdy_signal, ld_wb0_ren_signal;
  sc_vector<sc_signal<data_vector<tb_input_precision>>> ld_wb0_rdata_signal;
  sc_vector<sc_vector<sc_signal<bool>>> ld_wb1_rdy_signal, ld_wb1_ren_signal;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> ld_wb1_rdata_signal;
  sc_vector<sc_vector<sc_signal<bool>>> evrf_wb1_ren_signal, mfu0_wb1_ren_signal, mfu1_wb1_ren_signal;

  // Macro-op signals are defined as a vector of size 1 to match the same template definition of AXI-streaming
  // interfaces working on multiple cores
  sc_vector<sc_vector<sc_signal<bool>>> evrf_mop_rdy_signal, evrf_mop_ren_signal;
  sc_vector<sc_vector<sc_signal<evrf_mop>>> evrf_mop_rdata_signal;
  sc_vector<sc_signal<bool>> evrf_uop_rdy_signal, evrf_uop_wen_signal;
  sc_vector<sc_signal<evrf_uop>> evrf_uop_wdata_signal;
  sc_vector<sc_vector<sc_signal<bool>>> mfu0_mop_rdy_signal, mfu0_mop_ren_signal;
  sc_vector<sc_vector<sc_signal<mfu_mop>>> mfu0_mop_rdata_signal;
  sc_vector<sc_signal<bool>> mfu0_uop_rdy_signal, mfu0_uop_wen_signal;
  sc_vector<sc_signal<mfu_uop>> mfu0_uop_wdata_signal;
  sc_vector<sc_vector<sc_signal<bool>>> mfu1_mop_rdy_signal, mfu1_mop_ren_signal;
  sc_vector<sc_vector<sc_signal<mfu_mop>>> mfu1_mop_rdata_signal;
  sc_vector<sc_signal<bool>> mfu1_uop_rdy_signal, mfu1_uop_wen_signal;
  sc_vector<sc_signal<mfu_uop>> mfu1_uop_wdata_signal;
  sc_vector<sc_signal<bool>> ld_mop_rdy_signal, ld_mop_ren_signal;
  sc_vector<sc_signal<ld_mop>> ld_mop_rdata_signal;
  sc_signal<bool> ld_uop_rdy_signal, ld_uop_wen_signal;
  sc_signal<ld_uop> ld_uop_wdata_signal;

 public:
  sc_in<bool> rst;
  // Instruction interfaces
  sc_vector<axis_slave_port> evrf_inst_interfaces;
  sc_vector<axis_slave_port> mfu0_inst_interfaces;
  sc_vector<axis_slave_port> mfu1_inst_interfaces;
  axis_slave_port loader_inst_interface;
  // Input and write-back interfaces
  sc_vector<axis_slave_port> evrf_ififo_interfaces;
  axis_master_port loader_wb0_interface;
  // Loader external inputs/outputs
  sc_vector<sc_out<bool>> ext_input_fifo_rdy;
  sc_vector<sc_in<bool>> ext_input_fifo_wen;
  sc_vector<sc_in<data_vector<tb_input_precision>>> ext_input_fifo_wdata;
  sc_vector<sc_out<bool>> ext_output_fifo_rdy;
  sc_vector<sc_in<bool>> ext_output_fifo_ren;
  sc_vector<sc_out<data_vector<tb_output_precision>>> ext_output_fifo_rdata;

  axis_vector_elementwise(const sc_module_name& name, unsigned int thread_id);
  ~axis_vector_elementwise();
  void RegisterModuleInfo();

  void Assign();
  SC_HAS_PROCESS(axis_vector_elementwise);
};