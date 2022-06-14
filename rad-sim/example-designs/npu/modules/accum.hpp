#pragma once

#include <systemc.h>

#include <instructions.hpp>
#include <params.hpp>
#include <sim_utils.hpp>

class accum : public sc_module {
 private:
  std::vector<std::vector<std::vector<tb_output_precision>>> accum_scratchpad;
  sc_signal<sc_uint<ACCUMIDW>> accum_raddr;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> accum_in_pipeline;
  sc_vector<sc_signal<sc_uint<ACCUMIDW>>> accum_raddr_pipeline;
  sc_vector<sc_signal<bool>> valid_in_pipeline;
  sc_vector<sc_signal<sc_uint<2>>> accum_op_pipeline;
  sc_vector<sc_signal<data_vector<tb_output_precision>>> accum_wdata;
  sc_vector<sc_signal<data_vector<tb_output_precision>>> accum_rdata;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_in<bool> valid_in;
  sc_in<sc_uint<ACCUMIDW>> accum_size;
  sc_in<sc_uint<2>> accum_op;
  sc_vector<sc_in<data_vector<tb_output_precision>>> accum_in;
  sc_out<bool> valid_out;
  sc_vector<sc_out<data_vector<tb_output_precision>>> accum_out;
  sc_in<bool> clk_en;

  accum(const sc_module_name& name);
  ~accum();

  void Tick();
  void Assign();

  SC_HAS_PROCESS(accum);
};
