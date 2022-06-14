#pragma once

#include <systemc.h>

#include <fifo.hpp>
#include <instruction_fifo.hpp>
#include <instructions.hpp>
#include <params.hpp>
#include <register_file.hpp>
#include <sim_utils.hpp>
#include <radsim_telemetry.hpp>

// This class defines the multi-function unit module which implements vector elementwise operations in the NPU. The
// current MFU implements the following operations: activation functions (ReLU, sigmoid, tanh), addition operations
// (A+B, A-B, B-A, max(A,B)), and multiplication operation
class mfu : public sc_module {
 private:
  unsigned int mfu_id;  // MFU ID to differentiate between different MFUs (if multiple exist)
  unsigned int sector_id;
  unsigned int thread_id;

  // Sub-modules
  instruction_fifo<mfu_uop>* inst_fifo;  // MFU instruction FIFO
  std::vector<fifo<tb_output_precision>*> ififos;
  std::vector<fifo<tb_output_precision>*> wb_ififos;
  std::vector<fifo<tb_output_precision>*> ofifos;                        // MFU output FIFOs (one per core)
  std::vector<register_file<tb_output_precision>*> add_register_files;   // Vector register files for addition unit
  std::vector<register_file<tb_output_precision>*> mult_register_files;  // Vector register files for multiply unit

  // Control/data signals for different MFU FIFOs
  sc_signal<bool> inst_fifo_full, inst_fifo_almost_full, inst_fifo_empty, inst_fifo_almost_empty, inst_fifo_ren;
  sc_signal<mfu_uop> inst_fifo_rdata;
  sc_vector<sc_signal<bool>> ififo_full, ififo_almost_full, ififo_empty, ififo_almost_empty, ififo_ren, ififo_wen;
  sc_vector<sc_signal<data_vector<tb_output_precision>>> ififo_rdata;
  sc_vector<sc_signal<bool>> wb_ififo_full, wb_ififo_almost_full, wb_ififo_empty, wb_ififo_almost_empty, wb_ififo_ren,
      wb_ififo_wen;
  sc_vector<sc_signal<data_vector<tb_output_precision>>> wb_ififo_rdata;
  sc_vector<sc_signal<bool>> ofifo_full, ofifo_almost_full, ofifo_empty, ofifo_almost_empty;
  sc_signal<mfu_uop> reset_uop;  // Dummy instruction signal for resetting the pipeline initially

  // Address, control, and data signals for addition and multiplication register files
  sc_signal<unsigned int> add_vrf_raddr_signal, mult_vrf_raddr_signal;
  sc_vector<sc_signal<unsigned int>> add_vrf_waddr_signal, mult_vrf_waddr_signal;
  sc_vector<sc_signal<data_vector<tb_output_precision>>> add_vrf_wdata_signal, mult_vrf_wdata_signal,
      add_vrf_rdata_signal, mult_vrf_rdata_signal;
  sc_vector<sc_signal<bool>> add_vrf_wen_signal, mult_vrf_wen_signal;
  sc_signal<bool> vrf_clk_en;  // Active low VRF clock enable signal

  // Signal for keeping track of the current tag value (should dispatch instruction if their tag is less than or equal
  // to this value)
  sc_signal<sc_uint<TAGW>> current_tag;

  // Pipelines to model delay of the MFU
  sc_vector<sc_signal<mfu_uop>> inst_pipeline;
  sc_vector<sc_signal<bool>> inst_valid_pipeline;
  // Data pipeline from input to compute units
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> input_to_compute_data_pipeline;
  // Data pipeline from addition and multiplication VRFs to compute units
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> add_vrf_to_compute_data_pipeline;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> mult_vrf_to_compute_data_pipeline;
  // Data pipeline for modeling the delay of the compute units themselves
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> compute_data_pipeline;
  // Data pipeline for modeling the delay from the compute units to the output FIFOs
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> compute_to_ofifo_data_pipeline;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  // Instruction Interface
  sc_in<mfu_uop> inst;
  sc_in<bool> inst_fifo_wen;
  sc_out<bool> inst_fifo_rdy;
  // VRF Write Interface
  sc_vector<sc_in<bool>> wbfifo_rdy;
  sc_vector<sc_out<bool>> wbfifo_ren;
  sc_vector<sc_in<data_vector<tb_output_precision>>> wbfifo_rdata;
  // Input Interface
  sc_vector<sc_in<bool>> input_ofifo_rdy;
  sc_vector<sc_out<bool>> input_ofifo_ren;
  sc_vector<sc_in<data_vector<tb_output_precision>>> input_ofifo_rdata;
  // Output Interface
  sc_vector<sc_out<bool>> mfu_ofifo_rdy;
  sc_vector<sc_in<bool>> mfu_ofifo_ren;
  sc_vector<sc_out<data_vector<tb_output_precision>>> mfu_result;

  mfu(const sc_module_name& name, unsigned int id, unsigned int id_sector, unsigned int id_thread);
  ~mfu();

  void Tick();
  void Assign();
  SC_HAS_PROCESS(mfu);
};