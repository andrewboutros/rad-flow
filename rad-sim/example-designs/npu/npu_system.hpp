#pragma once

#include <systemc.h>

#include <chrono>
#include <instructions.hpp>
#include <npu_top.hpp>
#include <npu_driver.hpp>
#include <params.hpp>
#include <sim_utils.hpp>
#include <vector>
#include <design_system.hpp>

class npu_system : public RADSimDesignSystem {
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

  sc_signal<bool> rst_sig;

  npu_driver* npu_driver_inst;
  npu_top* npu_inst;

  npu_system(const sc_module_name& name, sc_clock* driver_clk_sig, RADSimDesignContext* radsim_design);
  ~npu_system();
};