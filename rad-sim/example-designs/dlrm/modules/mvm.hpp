#pragma once

#include <axis_interface.hpp>
#include <design_context.hpp>
#include <fifo.hpp>
#include <instructions.hpp>
#include <radsim_defines.hpp>
#include <register_file.hpp>
#include <string>
#include <systemc.h>
#include <vector>

#define BITWIDTH 16
#define LANES 16
#define FIFO_SIZE 512
#define COMPUTE_LATENCY 8
#define MEM_DEPTH 1025
#define DOT_PRODUCTS LANES
#define DATAW (BITWIDTH * LANES)

class mvm : public radsim_module {
private:
  std::string module_name;
  unsigned int mvm_id;
  unsigned int layer_id;

  std::vector<mvm_inst> inst_memory;
  sc_signal<mvm_inst> next_inst;
  sc_signal<unsigned int> pc;

  std::vector<data_vector<int16_t>> accum_memory;
  sc_signal<data_vector<int16_t>> next_accum;

  std::vector<register_file<int16_t> *> matrix_memory;
  sc_vector<sc_signal<data_vector<int16_t>>> matrix_mem_rdata;
  sc_signal<data_vector<int16_t>> matrix_mem_wdata;
  sc_vector<sc_signal<bool>> matrix_mem_wen;
  sc_signal<unsigned int> matrix_mem_raddr, matrix_mem_waddr;
  sc_signal<bool> matrix_mem_clk_en;

  fifo<int16_t> *ififo;
  sc_signal<data_vector<int16_t>> ififo_wdata_signal, ififo_rdata_signal;
  sc_signal<bool> ififo_wen_signal, ififo_ren_signal, ififo_full_signal,
      ififo_empty_signal, ififo_almost_full_signal, ififo_almost_empty_signal;

  fifo<int16_t> *reduce_fifo;
  sc_signal<data_vector<int16_t>> reduce_fifo_wdata_signal,
      reduce_fifo_rdata_signal;
  sc_signal<bool> reduce_fifo_wen_signal, reduce_fifo_ren_signal,
      reduce_fifo_full_signal, reduce_fifo_empty_signal,
      reduce_fifo_almost_full_signal, reduce_fifo_almost_empty_signal;

  sc_vector<sc_signal<data_vector<int16_t>>> ififo_pipeline, reduce_pipeline,
      result_pipeline;
  sc_vector<sc_signal<bool>> valid_pipeline, release_pipeline,
      accum_en_pipeline;
  sc_vector<sc_signal<unsigned int>> accum_pipeline;
  sc_vector<sc_signal<sc_int<5>>> dest_layer_pipeline;
  sc_vector<sc_signal<sc_uint<5>>> dest_mvm_pipeline;

  fifo<int16_t> *ofifo;
  sc_signal<data_vector<int16_t>> ofifo_wdata_signal, ofifo_rdata_signal;
  sc_signal<bool> ofifo_wen_signal, ofifo_ren_signal, ofifo_full_signal,
      ofifo_empty_signal, ofifo_almost_full_signal, ofifo_almost_empty_signal;

  fifo<sc_int<5>> *dl_fifo;
  sc_signal<data_vector<sc_int<5>>> dl_fifo_wdata_signal, dl_fifo_rdata_signal;
  sc_signal<bool> dl_fifo_wen_signal, dl_fifo_ren_signal, dl_fifo_full_signal,
      dl_fifo_empty_signal, dl_fifo_almost_full_signal,
      dl_fifo_almost_empty_signal;

  fifo<sc_uint<5>> *dm_fifo;
  sc_signal<data_vector<sc_uint<5>>> dm_fifo_wdata_signal, dm_fifo_rdata_signal;
  sc_signal<bool> dm_fifo_wen_signal, dm_fifo_ren_signal, dm_fifo_full_signal,
      dm_fifo_empty_signal, dm_fifo_almost_full_signal,
      dm_fifo_almost_empty_signal;

  data_vector<int16_t> tdata_vec;
  data_vector<int16_t> result;
  sc_signal<bool> dot_op, dot_reduce_op;

public:
  sc_in<bool> rst;
  axis_slave_port rx_input_interface;
  axis_slave_port rx_reduce_interface;
  axis_master_port tx_input_interface;
  axis_master_port tx_reduce_interface;

  mvm(const sc_module_name &name, unsigned int id_mvm, unsigned int id_layer,
      const std::string &inst_filename);
  ~mvm();

  void Assign();
  void Tick();
  SC_HAS_PROCESS(mvm);
  void RegisterModuleInfo();
};