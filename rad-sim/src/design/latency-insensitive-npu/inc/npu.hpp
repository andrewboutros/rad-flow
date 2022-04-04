#pragma once

#include <systemc.h>

#include <axis_evrf.hpp>
#include <axis_inst_dispatch.hpp>
#include <axis_loader.hpp>
#include <axis_mfu.hpp>
#include <axis_mvu_sector.hpp>
#include <axis_mvu_sector_chain.hpp>
#include <axis_switch.hpp>
#include <instructions.hpp>
#include <params.hpp>
#include <sim_utils.hpp>
#include <unordered_map>

class npu : public sc_module {
 private:
  // Sub-modules
  axis_mvu_sector* first_mvu_sector;
  std::vector<axis_mvu_sector_chain*> mvu_sectors;
  std::vector<std::vector<axis_evrf*>> evrf_blocks;
  std::vector<std::vector<axis_mfu*>> mfu0_blocks;
  std::vector<std::vector<axis_mfu*>> mfu1_blocks;
  std::vector<axis_loader*> ld_blocks;
  std::vector<std::vector<axis_switch*>> wb_switches;
  std::vector<axis_inst_dispatch*> inst_dispatcher;

  sc_vector<sc_vector<axis_signal>> sectors_inst_signals;
  sc_vector<sc_vector<axis_signal>> evrf_inst_signals;
  sc_vector<sc_vector<axis_signal>> mfu0_inst_signals;
  sc_vector<sc_vector<axis_signal>> mfu1_inst_signals;
  sc_vector<axis_signal> ld_inst_signals;

  sc_vector<sc_vector<axis_signal>> sector_to_evrf_signals;
  sc_vector<sc_vector<axis_signal>> evrf_to_mfu0_axis_signals;
  sc_vector<sc_vector<axis_signal>> mfu0_to_mfu1_axis_signals;
  sc_vector<sc_vector<axis_signal>> mfu1_to_ld_axis_signals;
  sc_vector<axis_signal> ld_wb0_axis_signals;
  sc_vector<sc_vector<axis_signal>> ld_wb1_axis_signals;
  sc_vector<sc_vector<sc_vector<axis_signal>>> wb_axis_signals;

  sc_vector<sc_vector<sc_vector<sc_signal<bool>>>> sector_chain_fifo_rdy_signals;
  sc_vector<sc_vector<sc_vector<sc_signal<bool>>>> sector_chain_fifo_ren_signals;
  sc_vector<sc_vector<sc_vector<sc_signal<data_vector<tb_input_precision>>>>> sector_chain_fifo_rdata_signals;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  // Instruction Interface
  sc_in<vliw_inst> inst_wdata;
  sc_in<unsigned int> inst_waddr;
  sc_in<bool> inst_wen;
  sc_in<unsigned int> start_pc;
  sc_in<unsigned int> end_pc;
  sc_in<bool> start;
  // MRF Write Interface
  sc_in<sc_uint<MRF_ADDRW>> mrf_waddr;
  sc_in<data_vector<tb_input_precision>> mrf_wdata;
  sc_in<sc_uint<MRFIDW>> mrf_wid;
  // Input Interface
  sc_vector<sc_vector<sc_out<bool>>> ififo_rdy;
  sc_vector<sc_vector<sc_in<bool>>> ififo_wen;
  sc_vector<sc_vector<sc_in<data_vector<tb_input_precision>>>> ififo_wdata;
  // Output Interface
  sc_vector<sc_vector<sc_out<bool>>> ofifo_rdy;
  sc_vector<sc_vector<sc_in<bool>>> ofifo_ren;
  sc_vector<sc_vector<sc_out<data_vector<tb_output_precision>>>> ofifo_rdata;

  // Tile constructor and destructor
  explicit npu(const sc_module_name& name);
  ~npu();

  void Assign();

  SC_HAS_PROCESS(npu);
};
