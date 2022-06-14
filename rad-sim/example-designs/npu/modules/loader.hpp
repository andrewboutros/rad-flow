#pragma once

#include <systemc.h>

#include <fifo.hpp>
#include <instruction_fifo.hpp>
#include <instructions.hpp>
#include <params.hpp>
#include <sim_utils.hpp>

// This class defines the NPU loader module which is responsible for writing the processor pipeline results or external
// inputs to any of the architecture states (i.e., vector register files) of the NPU, and sending NPU results to the
// outside world through an output FIFO interface
class loader : public sc_module {
 private:
  // Sub-modules
  instruction_fifo<ld_uop>* inst_fifo;                       // Loader instruction (micro-op) FIFO
  std::vector<fifo<tb_output_precision>*> ext_output_fifos;  // External output FIFOs (one per core)
  std::vector<fifo<tb_input_precision>*> ext_input_fifos;    // External input FIFOs (one per core)
  // Two channels of write-back FIFOs (each has a FIFO per core) for writing back the NPU results to two distinct
  // destinations (i.e., register files) at the same time
  std::vector<fifo<tb_input_precision>*> wb0_fifos;
  std::vector<std::vector<fifo<tb_output_precision>*>> wb1_fifos;
  std::vector<std::vector<fifo<tb_output_precision>*>> ififos;

  // Control and data signals for input, output and instruction FIFOs
  sc_signal<bool> inst_fifo_full, inst_fifo_almost_full, inst_fifo_empty, inst_fifo_almost_empty, inst_fifo_ren;
  sc_signal<ld_uop> inst_fifo_rdata;
  sc_vector<sc_signal<bool>> ext_input_fifo_full, ext_input_fifo_almost_full, ext_input_fifo_empty,
      ext_input_fifo_almost_empty, ext_input_fifo_ren;
  sc_vector<sc_signal<data_vector<tb_input_precision>>> ext_input_fifo_rdata;
  sc_vector<sc_signal<bool>> ext_output_fifo_full, ext_output_fifo_almost_full, ext_output_fifo_empty,
      ext_output_fifo_almost_empty;

  // Control and data signals for write-back FIFOs
  sc_vector<sc_signal<bool>> wb0_fifo_full, wb0_fifo_almost_full, wb0_fifo_empty, wb0_fifo_almost_empty;
  sc_vector<sc_vector<sc_signal<bool>>> wb1_fifo_full, wb1_fifo_almost_full, wb1_fifo_empty, wb1_fifo_almost_empty;
  sc_vector<sc_signal<data_vector<tb_input_precision>>> wb0_fifo_wdata;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> wb1_fifo_wdata;
  sc_vector<sc_vector<sc_signal<bool>>> ififo_full, ififo_almost_full, ififo_empty, ififo_almost_empty, ififo_wen,
      ififo_ren;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> ififo_rdata;

  // Dummy (default/invalid) instruction for reseting the instruction pipeline
  sc_signal<ld_uop> reset_uop;

  // Pipelines to model the delay of the loader module
  sc_vector<sc_signal<ld_uop>> inst_pipeline;
  sc_vector<sc_signal<bool>> inst_valid_pipeline, ext_output_fifo_wen_pipeline, wb0_fifo_wen_pipeline,
      wb1_fifo_wen_pipeline;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> data_pipeline;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  // Instruction Interface
  sc_in<ld_uop> inst;
  sc_in<bool> inst_fifo_wen;
  sc_out<bool> inst_fifo_rdy;
  // VRF Write Ch0 Interface
  sc_vector<sc_out<bool>> wb0_fifo_rdy;
  sc_vector<sc_in<bool>> wb0_fifo_ren;
  sc_vector<sc_out<data_vector<tb_input_precision>>> wb0_fifo_rdata;
  // VRF Write Ch1 Interface
  sc_vector<sc_vector<sc_out<bool>>> wb1_fifo_rdy;
  sc_vector<sc_vector<sc_in<bool>>> wb1_fifo_ren;
  sc_vector<sc_vector<sc_out<data_vector<tb_output_precision>>>> wb1_fifo_rdata;
  // Write-back Interface
  sc_vector<sc_vector<sc_in<bool>>> input_fifo_rdy;
  sc_vector<sc_vector<sc_out<bool>>> input_fifo_ren;
  sc_vector<sc_vector<sc_in<data_vector<tb_output_precision>>>> input_fifo_rdata;
  // Input Interface
  sc_vector<sc_out<bool>> ext_input_fifo_rdy;
  sc_vector<sc_in<bool>> ext_input_fifo_wen;
  sc_vector<sc_in<data_vector<tb_input_precision>>> ext_input_fifo_wdata;
  // Output Interface
  sc_vector<sc_out<bool>> ext_output_fifo_rdy;
  sc_vector<sc_in<bool>> ext_output_fifo_ren;
  sc_vector<sc_out<data_vector<tb_output_precision>>> ext_output_fifo_rdata;

  // Tile constructor and destructor
  loader(const sc_module_name& name);
  ~loader();

  void Tick();
  void Assign();
  SC_HAS_PROCESS(loader);
};