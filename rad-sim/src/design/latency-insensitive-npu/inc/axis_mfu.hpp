#pragma once

#include <systemc.h>

#include <axis_fifo_adapters.hpp>
#include <axis_interface.hpp>
#include <decoders.hpp>
#include <mfu.hpp>

// This class defines the AXI-streaming wrapper for the multi-function unit module. It contains the MFU block and its
// instruction decoder, as well as AXI-streaming interfaces for all their input/output FIFOs
class axis_mfu : public sc_module {
 private:
  unsigned int mfu_id;  // Multi-function unit ID to differentiate between different MFUs
  unsigned int sector_id;
  unsigned int thread_id;

  // Sub-modules
  mfu_decoder* decoder;  // MFU instruction decoder
  mfu* mfu_module;       // MFU module
  // Slave AXI-streaming interfaces for instructions and inputs, and master AXI-streaming interfaces for outputs
  axis_slave_fifo_adapter<mfu_mop, sc_bv<MFU_MOP_BITWIDTH>>* inst_axis_interface;
  axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>* ififo_axis_interface;
  axis_master_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>* ofifo_axis_interface;
  axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<NARROW_WRITEBACK_BV_WIDTH>>* wb_axis_interface;

  // Signals for connectiong different components of the wrapper
  sc_vector<sc_signal<bool>> ififo_rdy_signal, ififo_ren_signal;
  sc_vector<sc_signal<data_vector<tb_output_precision>>> ififo_rdata_signal;
  sc_vector<sc_signal<bool>> ofifo_rdy_signal, ofifo_ren_signal;
  sc_vector<sc_signal<data_vector<tb_output_precision>>> ofifo_rdata_signal;
  sc_vector<sc_signal<bool>> wb_rdy_signal, wb_ren_signal;
  sc_vector<sc_signal<data_vector<tb_output_precision>>> wb_rdata_signal;
  // Macro-op signals are defined as a vector of size 1 to match the same template definition of AXI-streaming
  // interfaces working on multiple cores
  sc_vector<sc_signal<bool>> mop_rdy_signal, mop_ren_signal;
  sc_vector<sc_signal<mfu_mop>> mop_rdata_signal;
  sc_signal<bool> uop_rdy_signal, uop_wen_signal;
  sc_signal<mfu_uop> uop_wdata_signal;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  axis_slave_port mfu_inst_interface;
  axis_slave_port mfu_wb_interface;
  axis_slave_port mfu_ififo_interface;
  axis_master_port mfu_ofifo_interface;

  axis_mfu(const sc_module_name& name, unsigned int id_mfu, unsigned int id_sector, unsigned int id_thread,
           unsigned int inst_interface_dataw, unsigned int wb_interface_dataw, unsigned int ififo_interface_dataw,
           unsigned int ofifo_interface_dataw);
  ~axis_mfu();
};