#pragma once

#include <systemc.h>

#include <chrono>
#include <instructions.hpp>
#include <design.hpp>
#include <params.hpp>
#include <sim_utils.hpp>
#include <vector>

class design_driver : public sc_module {
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

  design_driver(const sc_module_name& name);
  ~design_driver();

  void source();
  void sink();
  void assign();

  SC_HAS_PROCESS(design_driver);
};

class design_tb : public sc_module {
 private:
 public:
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

  sc_clock* noc_clk_sig;
  sc_clock* adapter_clk_sig;
  std::vector<sc_clock*> node_clks_sig;
  sc_signal<bool> rst_sig;

  design_driver* driver;
  design* rad_npu;

  design_tb(const sc_module_name& name, std::string& noc_config_filename);
  ~design_tb();
};