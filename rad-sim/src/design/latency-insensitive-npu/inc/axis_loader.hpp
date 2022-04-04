#pragma once

#include <systemc.h>

#include <axis_fifo_adapters.hpp>
#include <axis_interface.hpp>
#include <decoders.hpp>
#include <loader.hpp>

// This class defines the AXI-streaming wrapper for the loader module. It contains the loader block and its
// instruction decoder, as well as AXI-streaming interfaces for all their input/output FIFOs
class axis_loader : public sc_module {
 private:
  unsigned int thread_id;

  // Sub-modules
  ld_decoder* decoder;    // Loader instruction decoder
  loader* loader_module;  // Loader module
  // Slave AXI-streaming interfaces for instructions and inputs, and master AXI-streaming interfaces for outputs
  axis_slave_fifo_adapter<ld_mop, sc_bv<LD_MOP_BITWIDTH>>* inst_axis_interface;
  std::vector<axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>*>
      input_fifo_axis_interfaces;
  axis_master_fifo_adapter<data_vector<tb_input_precision>, sc_bv<WIDE_WRITEBACK_BV_WIDTH>>* wb0_axis_interface;
  std::vector<axis_master_fifo_adapter<data_vector<tb_output_precision>, sc_bv<NARROW_WRITEBACK_BV_WIDTH>>*>
      wb1_axis_interfaces;

  // Signals for connecting different components of the wrapper
  sc_vector<sc_vector<sc_signal<bool>>> input_fifo_rdy_signal, input_fifo_ren_signal;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> input_fifo_rdata_signal;
  sc_vector<sc_signal<bool>> wb0_rdy_signal, wb0_ren_signal;
  sc_vector<sc_signal<data_vector<tb_input_precision>>> wb0_rdata_signal;
  sc_vector<sc_vector<sc_signal<bool>>> wb1_rdy_signal, wb1_ren_signal;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> wb1_rdata_signal;
  // Macro-op signals are defined as a vector of size 1 to match the same template definition of AXI-streaming
  // interfaces working on multiple cores
  sc_vector<sc_signal<bool>> mop_rdy_signal, mop_ren_signal;
  sc_vector<sc_signal<ld_mop>> mop_rdata_signal;
  sc_signal<bool> uop_rdy_signal, uop_wen_signal;
  sc_signal<ld_uop> uop_wdata_signal;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_vector<sc_out<bool>> ext_input_fifo_rdy;
  sc_vector<sc_in<bool>> ext_input_fifo_wen;
  sc_vector<sc_in<data_vector<tb_input_precision>>> ext_input_fifo_wdata;
  sc_vector<sc_out<bool>> ext_output_fifo_rdy;
  sc_vector<sc_in<bool>> ext_output_fifo_ren;
  sc_vector<sc_out<data_vector<tb_output_precision>>> ext_output_fifo_rdata;
  axis_slave_port loader_inst_interface;
  sc_vector<axis_slave_port> loader_input_fifo_interfaces;
  axis_master_port loader_wb0_interface;
  sc_vector<axis_master_port> loader_wb1_interfaces;

  explicit axis_loader(const sc_module_name& name, unsigned int inst_interface_dataw, unsigned int wb0_interface_dataw,
                       unsigned int wb1_interface_dataw, unsigned int ififo_interface_dataw, unsigned int id);
  ~axis_loader();
};