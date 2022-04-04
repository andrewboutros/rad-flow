#pragma once

#include <systemc.h>

#include <accum.hpp>
#include <asymmetric_fifo.hpp>
#include <fifo.hpp>
#include <instruction_fifo.hpp>
#include <instructions.hpp>
#include <params.hpp>
#include <piso_register_file.hpp>
#include <register_file.hpp>
#include <simplified_dpe.hpp>

class mvu_sector : public sc_module {
 private:
  unsigned int sector_id;

  std::vector<instruction_fifo<mvu_uop>*> inst_fifo;
  std::vector<std::vector<std::vector<simplified_dpe*>>> dpes;
  std::vector<std::vector<std::vector<piso_register_file<tb_input_precision>*>>> vrfs;
  std::vector<std::vector<register_file<tb_input_precision>*>> mrfs;
  accum* accumulator;
  std::vector<std::vector<fifo<tb_input_precision>*>> wb_ififos;
  std::vector<std::vector<fifo<tb_input_precision>*>> sector_chain_ofifos;
  std::vector<std::vector<asymmetric_fifo<tb_output_precision>*>> ofifos;

  sc_vector<sc_signal<bool>> inst_valid_pipeline;
  sc_vector<sc_signal<mvu_uop>> inst_pipeline;
  sc_vector<sc_signal<unsigned int>> mrf_raddr_pipeline;
  sc_vector<sc_vector<sc_vector<sc_signal<data_vector<tb_input_precision>>>>> vrf_rdata_pipeline;
  sc_vector<sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>>> dpe_result_signal;
  sc_vector<sc_vector<sc_vector<sc_signal<bool>>>> dpe_result_valid_signal;
  sc_signal<bool> dpe_valid_a_signal, dpe_valid_b_signal, dpe_reg_use_sel_signal, dpe_reg_shift_sel_signal;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> reduction_pipeline;
  sc_vector<sc_signal<bool>> reduction_valid_pipeline;
  sc_signal<sc_uint<ACCUMIDW>> accum_size_signal;
  sc_signal<sc_uint<2>> accum_op_signal;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> accum_to_ofifo_pipeline;
  sc_vector<sc_signal<bool>> accum_to_ofifo_valid_pipeline;

  sc_vector<sc_signal<bool>> inst_fifo_full, inst_fifo_almost_full, inst_fifo_empty, inst_fifo_almost_empty, inst_fifo_ren;
  sc_vector<sc_signal<mvu_uop>> inst_fifo_rdata;
  sc_vector<sc_vector<sc_signal<bool>>> ofifo_full, ofifo_almost_full, ofifo_empty, ofifo_almost_empty;
  sc_vector<sc_signal<bool>> ofifo_wen_signal;
  sc_vector<sc_vector<sc_signal<bool>>> wb_ififo_full, wb_ififo_almost_full, wb_ififo_empty,
      wb_ififo_almost_empty, wb_ififo_wen, wb_ififo_ren;
  sc_vector<sc_vector<sc_signal<data_vector<tb_input_precision>>>> wb_ififo_rdata;
  sc_vector<sc_vector<sc_signal<bool>>> sector_chain_ofifo_full, sector_chain_ofifo_almost_full, sector_chain_ofifo_empty,
      sector_chain_ofifo_almost_empty, sector_chain_ofifo_wen;
  sc_vector<sc_signal<bool>> sector_chain_ofifo_wen_signal;

  sc_signal<unsigned int> mrf_waddr_signal;
  sc_vector<sc_vector<sc_signal<bool>>> mrf_wen_signal;
  sc_vector<sc_vector<sc_signal<data_vector<tb_input_precision>>>> mrf_rdata_signal;

  sc_signal<unsigned int> vrf_raddr_signal;
  sc_signal<uint8_t> vrf_rd_id_signal;
  sc_vector<sc_vector<sc_vector<sc_signal<unsigned int>>>> vrf_waddr_signal;
  sc_vector<sc_vector<sc_vector<sc_signal<data_vector<tb_input_precision>>>>> vrf_wdata_signal;
  sc_vector<sc_vector<sc_vector<sc_signal<bool>>>> vrf_wen_signal;
  sc_vector<sc_vector<sc_vector<sc_signal<data_vector<tb_input_precision>>>>> vrf_rdata_signal;

  sc_vector<sc_signal<sc_uint<TAGW>>> current_tag;
  sc_signal<bool> sector_stall;
  sc_signal<unsigned int> active_thread;
  sc_vector<sc_signal<unsigned int>> thread_id_pipeline;
  sc_signal<bool> thread_locked;
  std::queue<unsigned int> thread_order;
  unsigned int count = 0;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  // Instruction Interface
  sc_vector<sc_in<mvu_uop>> inst;
  sc_vector<sc_in<bool>> inst_fifo_wen;
  sc_vector<sc_out<bool>> inst_fifo_rdy;
  // MRF Write Interface
  sc_in<sc_uint<MRF_ADDRW>> mrf_waddr;
  sc_in<data_vector<tb_input_precision>> mrf_wdata;
  sc_in<sc_uint<MRFIDW>> mrf_wid;
  // Write-back Interface
  sc_vector<sc_vector<sc_in<bool>>> wb_fifo_rdy;
  sc_vector<sc_vector<sc_out<bool>>> wb_fifo_ren;
  sc_vector<sc_vector<sc_in<data_vector<tb_input_precision>>>> wb_fifo_rdata;
  // Sector Chain Interface
  sc_vector<sc_vector<sc_out<bool>>> sector_chain_ofifo_rdy;
  sc_vector<sc_vector<sc_in<bool>>> sector_chain_ofifo_ren;
  sc_vector<sc_vector<sc_out<data_vector<tb_input_precision>>>> sector_chain_ofifo_rdata;
  // Output Interface
  sc_vector<sc_vector<sc_out<bool>>> sector_ofifo_rdy;
  sc_vector<sc_vector<sc_in<bool>>> sector_ofifo_ren;
  sc_vector<sc_vector<sc_out<data_vector<tb_output_precision>>>> sector_ofifo_rdata;

  mvu_sector(const sc_module_name& name, unsigned int id);
  ~mvu_sector();

  void Tick();
  void Assign();
  SC_HAS_PROCESS(mvu_sector);
};
