#pragma once

#include <systemc.h>

#include <chrono>
#include <instructions.hpp>
#include <npu_top.hpp>
#include <params.hpp>
#include <sim_utils.hpp>
#include <radsim_config.hpp>
#include <vector>

class npu_driver : public sc_module {
 private:
  int start_cycle, end_cycle;
  std::vector<vliw_inst> npu_program;
  std::vector<std::vector<tb_input_precision>> npu_inputs;
  std::vector<std::vector<tb_output_precision>> npu_outputs;

 public:
  sc_in<bool> clk;
  sc_out<bool> rst;
  sc_out<vliw_inst> inst_wdata;
  sc_out<unsigned int> inst_waddr;
  sc_out<bool> inst_wen;
  sc_out<unsigned int> start_pc;
  sc_out<unsigned int> end_pc;
  sc_out<bool> start;
  sc_out<sc_uint<MRF_ADDRW>> mrf_waddr;
  sc_out<data_vector<tb_input_precision>> mrf_wdata;
  sc_out<sc_uint<MRFIDW>> mrf_wid;
  sc_vector<sc_vector<sc_in<bool>>> ififo_rdy;
  sc_vector<sc_vector<sc_out<bool>>> ififo_wen;
  sc_vector<sc_vector<sc_out<data_vector<tb_input_precision>>>> ififo_wdata;
  sc_vector<sc_vector<sc_in<bool>>> ofifo_rdy;
  sc_vector<sc_vector<sc_out<bool>>> ofifo_ren;
  sc_vector<sc_vector<sc_in<data_vector<tb_output_precision>>>> ofifo_rdata;

  npu_driver(const sc_module_name& name);
  ~npu_driver();

  void source();
  void sink();
  void assign();

  SC_HAS_PROCESS(npu_driver);
};