#pragma once

#include <systemc.h>

#include <axis_fifo_adapters.hpp>
#include <axis_interface.hpp>
#include <decoders.hpp>
#include <mvu_sector.hpp>
#include <radsim_module.hpp>
#include <vector>

class axis_mvu_sector_chain : public radsim_module {
 private:
  std::string _name;
  unsigned int _sector_id;

  // Sub-modules
  std::vector<sector_and_accum_decoder*> sector_decoder;
  mvu_sector* sector_module;
  std::vector<axis_slave_fifo_adapter<mvu_mop, sc_bv<MVU_MOP_BITWIDTH>>*> inst_axis_interface;
  std::vector<axis_master_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>*>
      ofifo_axis_interface;

  // Signals for connecting different components of the wrapper
  sc_vector<sc_vector<sc_signal<bool>>> ofifo_rdy_signal, ofifo_ren_signal;
  sc_vector<sc_vector<sc_signal<data_vector<tb_output_precision>>>> ofifo_rdata_signal;
  // Macro-op signals are defined as a vector of size 1 to match the same template definition of AXI-streaming
  // interfaces working on multiple cores
  sc_vector<sc_vector<sc_signal<bool>>> mop_rdy_signal, mop_ren_signal;
  sc_vector<sc_vector<sc_signal<mvu_mop>>> mop_rdata_signal;
  sc_vector<sc_signal<bool>> uop_rdy_signal, uop_wen_signal;
  sc_vector<sc_signal<mvu_uop>> uop_wdata_signal;

 public:
  sc_in<bool> rst;
  sc_in<sc_uint<MRF_ADDRW>> mrf_waddr;
  sc_in<data_vector<tb_input_precision>> mrf_wdata;
  sc_in<sc_uint<MRFIDW>> mrf_wid;
  sc_vector<axis_slave_port> sector_inst_interface;
  sc_vector<sc_vector<sc_in<bool>>> wb_fifo_rdy;
  sc_vector<sc_vector<sc_out<bool>>> wb_fifo_ren;
  sc_vector<sc_vector<sc_in<data_vector<tb_input_precision>>>> wb_fifo_rdata;
  sc_vector<sc_vector<sc_out<bool>>> sector_chain_ofifo_rdy;
  sc_vector<sc_vector<sc_in<bool>>> sector_chain_ofifo_ren;
  sc_vector<sc_vector<sc_out<data_vector<tb_input_precision>>>> sector_chain_ofifo_rdata;
  sc_vector<axis_master_port> sector_ofifo_interface;

  axis_mvu_sector_chain(const sc_module_name& name, unsigned int id);
  ~axis_mvu_sector_chain();
  void RegisterModuleInfo();
};
