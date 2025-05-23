#pragma once

#include <systemc.h>
#include <vector>
#include <string>

#include "radsim_defines.hpp"
#include "axis_interface.hpp"
#include "fifo.hpp"
#include "register_file.hpp"
#include "instructions.hpp"

#include <design_context.hpp>

// In this implementation of the MVM, the number of lanes and dot products have to be set
// to the same value (i.e. native dimension). This ensures that the bitwidth of the input
// and output data are the same. The NATIVE_DIM parameter below has to match the native_dim
// parameter in the gen_testcase.py script. If you change this value, you have to also make
// sure that NATIVE_DIM * 32 (32b per element) is less than or equal the max_axis_tdata_width
// parameter in the config.yml file of the MLP example design
#define NATIVE_DIM 16
#define LANES NATIVE_DIM
#define DOT_PRODUCTS NATIVE_DIM

// Since the MVM is designed to have a single input AXI-S interface. The design can deadlock
// if the number of inputs per dispatcher is bigger than the FIFO_SIZE. If this condition is 
// not satisfied the input FIFO can get full and therefore block the reduction inputs from 
// reaching the reduce FIFO, resulting in a deadlock. This issue can be ultimately resolved
// by having different AXI-S input interfaces for input and reduce vectors with their traffic
// mapped to different NoC virtual channels. 
#define FIFO_SIZE 1024

// You can change the compute latency depending on the microarchitecture implementation of
// the MVM subcomponents (e.g. dot product engines with more lanes have higher latency
// as a result of more binary reduction tree levels)
#define COMPUTE_LATENCY 23

// Make sure that the memory depth is enough to hold the number of instructions and matrix
// weights for the simulated test case (can be checked in the .mif files generated by the
// gen_testcase.py script)
#define MEM_DEPTH 2048

class mvm : public RADSimModule {
 private:
  std::string module_name;
  unsigned int mvm_id;
  unsigned int layer_id;

  std::vector<mvm_inst> inst_memory;
  sc_signal<mvm_inst> next_inst;
  sc_signal<uint32_t> pc;

  std::vector<data_vector<sc_int<32>>> accum_memory;
  sc_signal<data_vector<sc_int<32>>> next_accum;

  std::vector<register_file<sc_int<32>>*> matrix_memory;
  sc_vector<sc_signal<data_vector<sc_int<32>>>> matrix_mem_rdata;
  sc_signal<data_vector<sc_int<32>>> matrix_mem_wdata;
  sc_vector<sc_signal<bool>> matrix_mem_wen;
  sc_signal<uint32_t> matrix_mem_raddr, matrix_mem_waddr;
  sc_signal<bool> matrix_mem_clk_en;

  fifo<sc_int<32>>* ififo;
  sc_signal<data_vector<sc_int<32>>> ififo_wdata_signal, ififo_rdata_signal;
  sc_signal<bool> ififo_wen_signal, ififo_ren_signal, ififo_full_signal, ififo_empty_signal, 
    ififo_almost_full_signal, ififo_almost_empty_signal;

  fifo<sc_int<32>>* reduce_fifo;
  sc_signal<data_vector<sc_int<32>>> reduce_fifo_wdata_signal, reduce_fifo_rdata_signal;
  sc_signal<bool> reduce_fifo_wen_signal, reduce_fifo_ren_signal, reduce_fifo_full_signal, 
    reduce_fifo_empty_signal, reduce_fifo_almost_full_signal, reduce_fifo_almost_empty_signal;

  sc_vector<sc_signal<data_vector<sc_int<32>>>> ififo_pipeline, reduce_pipeline, result_pipeline;
  sc_vector<sc_signal<bool>> valid_pipeline, release_pipeline, accum_en_pipeline;
  sc_vector<sc_signal<uint32_t>> accum_pipeline;
  sc_vector<sc_signal<sc_int<5>>> dest_layer_pipeline;
  sc_vector<sc_signal<sc_uint<5>>> dest_mvm_pipeline;

  fifo<sc_int<32>>* ofifo;
  sc_signal<data_vector<sc_int<32>>> ofifo_wdata_signal, ofifo_rdata_signal;
  sc_signal<bool> ofifo_wen_signal, ofifo_ren_signal, ofifo_full_signal, ofifo_empty_signal, 
    ofifo_almost_full_signal, ofifo_almost_empty_signal;

  fifo<sc_int<5>>* dl_fifo;
  sc_signal<data_vector<sc_int<5>>> dl_fifo_wdata_signal, dl_fifo_rdata_signal;
  sc_signal<bool> dl_fifo_wen_signal, dl_fifo_ren_signal, dl_fifo_full_signal, dl_fifo_empty_signal, 
    dl_fifo_almost_full_signal, dl_fifo_almost_empty_signal;

  fifo<sc_uint<5>>* dm_fifo;
  sc_signal<data_vector<sc_uint<5>>> dm_fifo_wdata_signal, dm_fifo_rdata_signal;
  sc_signal<bool> dm_fifo_wen_signal, dm_fifo_ren_signal, dm_fifo_full_signal, dm_fifo_empty_signal, 
    dm_fifo_almost_full_signal, dm_fifo_almost_empty_signal;
  
  data_vector<sc_int<32>> tdata_vec;
  data_vector<sc_int<32>> result;
  sc_signal<bool> dot_op, dot_reduce_op;

 public:
  RADSimDesignContext* radsim_design;
  sc_in<bool> rst;
  axis_slave_port rx_interface;
  axis_master_port tx_interface;

  mvm(const sc_module_name& name, unsigned int id_mvm, unsigned int id_layer, const std::string& inst_filename, RADSimDesignContext* radsim_design);
  ~mvm();

  void Assign();
  void Tick();
  SC_HAS_PROCESS(mvm);
  void RegisterModuleInfo();
};
