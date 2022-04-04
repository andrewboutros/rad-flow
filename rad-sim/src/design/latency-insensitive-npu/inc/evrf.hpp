#pragma once

#include <systemc.h>

#include <fifo.hpp>
#include <instruction_fifo.hpp>
#include <instructions.hpp>
#include <params.hpp>
#include <register_file.hpp>
#include <sim_utils.hpp>

// This class describes the external VRF module. This module contains one register file per NPU core and it selects
// either the register file contents or the output of the upstream module (in the baseline NPU, this is the accumulator
// module) and passes it to the downstream modules (rest of the processor pipeline -- MFUs and loader in baseline NPU)
class evrf : public sc_module {
 private:
  unsigned int sector_id;
  unsigned int thread_id;

  // Sub-modules
  instruction_fifo<evrf_uop>* inst_fifo;  // Instruction (micro-op) FIFO
  std::vector<fifo<tb_output_precision>*> ififos;
  std::vector<fifo<tb_output_precision>*> wb_ififos;
  std::vector<fifo<tb_output_precision>*> ofifos;  // Output FIFOs of the external VRF (one per NPU core)
  std::vector<register_file<tb_output_precision>*> register_files;  // Register files (one per NPU core)

  // Signals for instruction, input and output FIFOs
  sc_signal<bool> inst_fifo_full, inst_fifo_almost_full, inst_fifo_empty, inst_fifo_almost_empty, inst_fifo_ren;
  sc_signal<evrf_uop> inst_fifo_rdata;
  sc_vector<sc_signal<bool>> ififo_full, ififo_almost_full, ififo_empty, ififo_almost_empty, ififo_ren, ififo_wen;
  sc_vector<sc_signal<data_vector<tb_output_precision>>> ififo_rdata;
  sc_vector<sc_signal<bool>> wb_ififo_full, wb_ififo_almost_full, wb_ififo_empty, wb_ififo_almost_empty, wb_ififo_ren,
      wb_ififo_wen;
  sc_vector<sc_signal<data_vector<tb_output_precision>>> wb_ififo_rdata;
  sc_vector<sc_signal<bool>> ofifo_full, ofifo_almost_full, ofifo_empty, ofifo_almost_empty;
  sc_signal<evrf_uop> reset_uop;  // Dummy instruction signal for resetting the pipeline initially

  // Register file address and data signals -- one address & write enable for all RFs since they work in lockstep
  sc_vector<sc_signal<data_vector<tb_output_precision>>> vrf_wdata_signal, vrf_rdata_signal;
  sc_signal<unsigned int> vrf_raddr_signal, vrf_waddr_signal;
  sc_vector<sc_signal<bool>> vrf_wen_signal;
  sc_signal<bool> vrf_clk_en;  // Active-low clock enable for the RF

  // Signal for keeping track of the current tag value (should dispatch instruction if their tag is less than or equal
  // to this value)
  sc_signal<sc_uint<TAGW>> current_tag;

  // Pipelines to model the delay of the external VRF
  sc_vector<sc_signal<evrf_uop>> inst_pipeline;    // Instruction pipeline through the module
  sc_vector<sc_signal<bool>> inst_valid_pipeline;  // Valid instruction pipeline through the module
  // Input data pipeline to the selecting mux
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> input_data_pipeline;
  // Data pipeline from selecting mux to output FIFO
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> vrf_to_ofifo_data_pipeline;
  // Data valid pipeline from selecting mux to output FIFO
  sc_vector<sc_vector<sc_signal<bool>>> vrf_to_ofifo_valid_pipeline;

  unsigned int count = 0;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  // Instruction Interface
  sc_in<evrf_uop> inst;
  sc_in<bool> inst_fifo_wen;
  sc_out<bool> inst_fifo_rdy;
  // VRF Write Interface
  sc_vector<sc_in<bool>> wbfifo_rdy;
  sc_vector<sc_out<bool>> wbfifo_ren;
  sc_vector<sc_in<data_vector<tb_output_precision>>> wbfifo_rdata;
  // Input Interface
  sc_vector<sc_in<bool>> mvu_ofifo_rdy;
  sc_vector<sc_out<bool>> mvu_ofifo_ren;
  sc_vector<sc_in<data_vector<tb_output_precision>>> mvu_ofifo_rdata;
  // Output Interface
  sc_vector<sc_out<bool>> evrf_ofifo_rdy;
  sc_vector<sc_in<bool>> evrf_ofifo_ren;
  sc_vector<sc_out<data_vector<tb_output_precision>>> evrf_result;

  evrf(const sc_module_name& name, unsigned int id_sector, unsigned int id_thread);
  ~evrf();

  void Tick();
  void Assign();
  SC_HAS_PROCESS(evrf);
};