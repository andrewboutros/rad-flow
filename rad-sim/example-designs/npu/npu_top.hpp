#pragma once

#include <systemc.h>
#include <vector>


#include <axis_vector_elementwise.hpp>
#include <axis_inst_dispatch.hpp>
#include <axis_mvu_sector.hpp>
#include <axis_mvu_sector_chain.hpp>
#include <instructions.hpp>
#include <params.hpp>
#include <sim_utils.hpp>
#include <design_context.hpp>
#include <design_top.hpp>


class npu_top : public design_top {
 private:
  sc_vector<sc_vector<sc_vector<sc_signal<bool>>>> sector_chain_fifo_rdy_signals;
  sc_vector<sc_vector<sc_vector<sc_signal<bool>>>> sector_chain_fifo_ren_signals;
  sc_vector<sc_vector<sc_vector<sc_signal<data_vector<tb_input_precision>>>>> sector_chain_fifo_rdata_signals;

  axis_mvu_sector* first_mvu_sector;
  std::vector<axis_mvu_sector_chain*> mvu_sectors;
  std::vector<axis_vector_elementwise*> vector_elementwise_blocks;
  std::vector<axis_inst_dispatch*> inst_dispatcher;

 public:
  sc_in<bool> rst;
  // NPU instructions interface
  sc_in<vliw_inst> inst_wdata;
  sc_in<unsigned int> inst_waddr;
  sc_in<bool> inst_wen;
  sc_in<unsigned int> start_pc;
  sc_in<unsigned int> end_pc;
  sc_in<bool> start;
  // NPU MRF write interface
  sc_in<sc_uint<MRF_ADDRW>> mrf_waddr;
  sc_in<data_vector<tb_input_precision>> mrf_wdata;
  sc_in<sc_uint<MRFIDW>> mrf_wid;
  // NPU data input/output FIFO interface
  sc_vector<sc_vector<sc_out<bool>>> ififo_rdy;
  sc_vector<sc_vector<sc_in<bool>>> ififo_wen;
  sc_vector<sc_vector<sc_in<data_vector<tb_input_precision>>>> ififo_wdata;
  sc_vector<sc_vector<sc_out<bool>>> ofifo_rdy;
  sc_vector<sc_vector<sc_in<bool>>> ofifo_ren;
  sc_vector<sc_vector<sc_out<data_vector<tb_output_precision>>>> ofifo_rdata;

  npu_top(const sc_module_name& name, RADSimDesignContext* radsim_design);
  ~npu_top();
  void prepare_adapters_info();
};