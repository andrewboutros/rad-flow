#pragma once

#include <systemc.h>

#include <chrono>
#include <instructions.hpp>
#include <npu.hpp>
#include <params.hpp>
#include <sim_utils.hpp>

class npu_tb : public sc_module {
 private:
  int start_cycle, end_cycle;
  std::vector<vliw_inst> npu_program;
  std::vector<std::vector<tb_input_precision>> npu_inputs;
  std::vector<std::vector<tb_output_precision>> npu_outputs;

 public:
  sc_in<bool> clk;
  sc_out<bool> rst;
  // Instruction Interface
  sc_out<vliw_inst> inst_wdata;
  sc_out<unsigned int> inst_waddr;
  sc_out<bool> inst_wen;
  sc_out<unsigned int> start_pc;
  sc_out<unsigned int> end_pc;
  sc_out<bool> start;
  // MRF Write Interface
  sc_out<sc_uint<MRF_ADDRW>> mrf_waddr;
  sc_out<data_vector<tb_input_precision>> mrf_wdata;
  sc_out<sc_uint<MRFIDW>> mrf_wid;
  // Input Interface
  sc_vector<sc_vector<sc_in<bool>>> ififo_rdy;
  sc_vector<sc_vector<sc_out<bool>>> ififo_wen;
  sc_vector<sc_vector<sc_out<data_vector<tb_input_precision>>>> ififo_wdata;
  // Output Interface
  sc_vector<sc_vector<sc_in<bool>>> ofifo_rdy;
  sc_vector<sc_vector<sc_out<bool>>> ofifo_ren;
  sc_vector<sc_vector<sc_in<data_vector<tb_output_precision>>>> ofifo_rdata;

  npu_tb(const sc_module_name& name);
  ~npu_tb();

  void source();
  void sink();
  void assign();

  SC_HAS_PROCESS(npu_tb);
};

class npu_system : public sc_module {
 private:
 public:
  sc_signal<bool> rst_sig;
  sc_signal<vliw_inst> inst_wdata;
  sc_signal<unsigned int> inst_waddr;
  sc_signal<bool> inst_wen;
  sc_signal<unsigned int> start_pc, end_pc;
  sc_signal<bool> start;
  sc_signal<sc_uint<MRF_ADDRW>> mrf_waddr;
  sc_signal<data_vector<tb_input_precision>> mrf_wdata;
  sc_signal<sc_uint<MRFIDW>> mrf_wid;
  sc_vector<sc_vector<sc_signal<bool>>> ififo_rdy;
  sc_vector<sc_vector<sc_signal<bool>>> ififo_wen;
  sc_vector<sc_vector<sc_signal<data_vector<tb_input_precision>>>> ififo_wdata;
  sc_vector<sc_vector<sc_signal<bool>>> ofifo_rdy;
  sc_vector<sc_vector<sc_signal<bool>>> ofifo_ren;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> ofifo_rdata;

  sc_clock* clk_sig;
  npu_tb* _tb;
  npu* _npu;

  explicit npu_system(const sc_module_name& name);
  ~npu_system();
};
