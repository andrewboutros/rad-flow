#pragma once

#include <systemc.h>
#include <axis_interface.hpp>
#include <axis_fifo_adapters.hpp>
#include <decoders.hpp>
#include <radsim_module.hpp>
#include <vector>

// This class defines the AXI-streaming wrapper for the top-level instruction dispatcher. It contains the dispatcher
// block and AXI-streaming interfaces for all its output FIFOs
class axis_inst_dispatch : public radsim_module {
 private:
  std::string _name;
  unsigned int _thread_id;
  
  // Sub-modules
  vliw_inst_dispatch* inst_dispatch_module; // Top-level instruction dispatcher module
  // AXI-streaming master interfaces for instructions sent to different NPU blocks
  std::vector<axis_master_fifo_adapter<mvu_mop, sc_bv<MVU_MOP_BITWIDTH>>*> sector_mop_axis_interface;
  std::vector<axis_master_fifo_adapter<evrf_mop, sc_bv<EVRF_MOP_BITWIDTH>>*> evrf_mop_axis_interface;
  std::vector<axis_master_fifo_adapter<mfu_mop, sc_bv<MFU_MOP_BITWIDTH>>*> mfu0_mop_axis_interface;
  std::vector<axis_master_fifo_adapter<mfu_mop, sc_bv<MFU_MOP_BITWIDTH>>*> mfu1_mop_axis_interface;
  axis_master_fifo_adapter<ld_mop, sc_bv<LD_MOP_BITWIDTH>>* ld_mop_axis_interface;

  // Signals for connecting different components of the wrapper
  sc_vector<sc_vector<sc_signal<bool>>> sector_mop_rdy_signal, sector_mop_ren_signal;
  sc_vector<sc_vector<sc_signal<mvu_mop>>> sector_mop_rdata_signal;
  sc_vector<sc_vector<sc_signal<bool>>> evrf_mop_rdy_signal, evrf_mop_ren_signal;
  sc_vector<sc_vector<sc_signal<evrf_mop>>> evrf_mop_rdata_signal;
  sc_vector<sc_vector<sc_signal<bool>>> mfu0_mop_rdy_signal, mfu0_mop_ren_signal;
  sc_vector<sc_vector<sc_signal<mfu_mop>>> mfu0_mop_rdata_signal;
  sc_vector<sc_vector<sc_signal<bool>>> mfu1_mop_rdy_signal, mfu1_mop_ren_signal;
  sc_vector<sc_vector<sc_signal<mfu_mop>>> mfu1_mop_rdata_signal;
  sc_vector<sc_signal<bool>> ld_mop_rdy_signal, ld_mop_ren_signal;
  sc_vector<sc_signal<ld_mop>> ld_mop_rdata_signal;

 public:
  sc_in<bool> rst;
  sc_in<unsigned int> start_pc;
  sc_in<unsigned int> end_pc;
  sc_in<bool> start;
  sc_in<vliw_inst> inst_wdata;
  sc_in<unsigned int> inst_waddr;
  sc_in<bool> inst_wen;
  sc_vector<axis_master_port> sector_mop_interface;
  sc_vector<axis_master_port> evrf_mop_interface;
  sc_vector<axis_master_port> mfu0_mop_interface;
  sc_vector<axis_master_port> mfu1_mop_interface;
  axis_master_port ld_mop_interface;

  axis_inst_dispatch(const sc_module_name& name, unsigned int thread_id);
  ~axis_inst_dispatch();
  void RegisterModuleInfo();
};
