#include <axis_inst_dispatch.hpp>

axis_inst_dispatch::axis_inst_dispatch(const sc_module_name& name, unsigned int mop_interface_dataw, unsigned int id)
    : sc_module(name) {
  // Create SystemC vectors with the required sizes -- macro-op interfaces are vectors of size 1 to match the template
  // definition used with data FIFOs which works with multiple cores
  init_vector<axis_master_port>::init_sc_vector(sector_mop_interface, SECTORS);
  init_vector<axis_master_port>::init_sc_vector(evrf_mop_interface, SECTORS);
  init_vector<axis_master_port>::init_sc_vector(mfu0_mop_interface, SECTORS);
  init_vector<axis_master_port>::init_sc_vector(mfu1_mop_interface, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_mop_rdy_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_mop_ren_signal, SECTORS, 1);
  init_vector<sc_signal<mvu_mop>>::init_sc_vector(sector_mop_rdata_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_mop_rdy_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_mop_ren_signal, SECTORS, 1);
  init_vector<sc_signal<evrf_mop>>::init_sc_vector(evrf_mop_rdata_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_mop_rdy_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_mop_ren_signal, SECTORS, 1);
  init_vector<sc_signal<mfu_mop>>::init_sc_vector(mfu0_mop_rdata_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_mop_rdy_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_mop_ren_signal, SECTORS, 1);
  init_vector<sc_signal<mfu_mop>>::init_sc_vector(mfu1_mop_rdata_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(ld_mop_rdy_signal, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(ld_mop_ren_signal, 1);
  init_vector<sc_signal<ld_mop>>::init_sc_vector(ld_mop_rdata_signal, 1);

  thread_id = id;

  // Create instruction dispatcher module
  inst_dispatch_module = new vliw_inst_dispatch("inst_dispatcher");
  inst_dispatch_module->clk(clk);
  inst_dispatch_module->rst(rst);
  inst_dispatch_module->start_pc(start_pc);
  inst_dispatch_module->end_pc(end_pc);
  inst_dispatch_module->start(start);
  inst_dispatch_module->inst_wdata(inst_wdata);
  inst_dispatch_module->inst_waddr(inst_waddr);
  inst_dispatch_module->inst_wen(inst_wen);
  inst_dispatch_module->sector_mop_rdy(sector_mop_rdy_signal);
  inst_dispatch_module->sector_mop_ren(sector_mop_ren_signal);
  inst_dispatch_module->sector_mop_rdata(sector_mop_rdata_signal);
  inst_dispatch_module->evrf_mop_rdy(evrf_mop_rdy_signal);
  inst_dispatch_module->evrf_mop_ren(evrf_mop_ren_signal);
  inst_dispatch_module->evrf_mop_rdata(evrf_mop_rdata_signal);
  inst_dispatch_module->mfu0_mop_rdy(mfu0_mop_rdy_signal);
  inst_dispatch_module->mfu0_mop_ren(mfu0_mop_ren_signal);
  inst_dispatch_module->mfu0_mop_rdata(mfu0_mop_rdata_signal);
  inst_dispatch_module->mfu1_mop_rdy(mfu1_mop_rdy_signal);
  inst_dispatch_module->mfu1_mop_ren(mfu1_mop_ren_signal);
  inst_dispatch_module->mfu1_mop_rdata(mfu1_mop_rdata_signal);
  inst_dispatch_module->ld_mop_rdy(ld_mop_rdy_signal);
  inst_dispatch_module->ld_mop_ren(ld_mop_ren_signal);
  inst_dispatch_module->ld_mop_rdata(ld_mop_rdata_signal);

  // Create master AXI-streaming interfaces (send instructions to different NPU blocks)
  char module_name[NAME_LENGTH];
  std::string module_name_str;
  std::string dest_name;
  unsigned int dest_node;

  sector_mop_axis_interface.resize(SECTORS);
  evrf_mop_axis_interface.resize(SECTORS);
  mfu0_mop_axis_interface.resize(SECTORS);
  mfu1_mop_axis_interface.resize(SECTORS);

  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
#ifdef RADSIM
    dest_name = "axis_mvu_sector_" + std::to_string(sector_id);
    dest_node = module_assignment[dest_name];
#else
    dest_node = (sector_id * 6) + SECTOR_DEST_ID;
#endif
    module_name_str = "sector_" + std::to_string(sector_id) + "_mop_axis_interface_" + std::to_string(thread_id);
    std::strcpy(module_name, module_name_str.c_str());
    sector_mop_axis_interface[sector_id] = new axis_master_fifo_adapter<mvu_mop, sc_bv<MVU_MOP_BITWIDTH>>(
        module_name, inst, mop_interface_dataw, 1, MVU_MOP_BITWIDTH, dest_node, SECTOR_INST_STREAM_ID + thread_id);
    sector_mop_axis_interface[sector_id]->clk(clk);
    sector_mop_axis_interface[sector_id]->rst(rst);
    sector_mop_axis_interface[sector_id]->fifo_rdy(sector_mop_rdy_signal[sector_id]);
    sector_mop_axis_interface[sector_id]->fifo_ren(sector_mop_ren_signal[sector_id]);
    sector_mop_axis_interface[sector_id]->fifo_rdata(sector_mop_rdata_signal[sector_id]);
    sector_mop_axis_interface[sector_id]->axis_port.ConnectToPort(sector_mop_interface[sector_id]);

#ifdef RADSIM
    dest_name = "axis_vector_elementwise_" + std::to_string(thread_id) + "_" + std::to_string(sector_id);
    dest_node = module_assignment[dest_name];
#else
    dest_node = (sector_id * 6) + EVRF_DEST_ID;
#endif
    module_name_str = "evrf_" + std::to_string(sector_id) + "_mop_axis_interface_" + std::to_string(thread_id);
    std::strcpy(module_name, module_name_str.c_str());
    evrf_mop_axis_interface[sector_id] = new axis_master_fifo_adapter<evrf_mop, sc_bv<EVRF_MOP_BITWIDTH>>(
        module_name, inst, mop_interface_dataw, 1, EVRF_MOP_BITWIDTH, dest_node, EVRF_INST_STREAM_ID);
    evrf_mop_axis_interface[sector_id]->clk(clk);
    evrf_mop_axis_interface[sector_id]->rst(rst);
    evrf_mop_axis_interface[sector_id]->fifo_rdy(evrf_mop_rdy_signal[sector_id]);
    evrf_mop_axis_interface[sector_id]->fifo_ren(evrf_mop_ren_signal[sector_id]);
    evrf_mop_axis_interface[sector_id]->fifo_rdata(evrf_mop_rdata_signal[sector_id]);
    evrf_mop_axis_interface[sector_id]->axis_port.ConnectToPort(evrf_mop_interface[sector_id]);

#ifdef RADSIM
    dest_name = "axis_vector_elementwise_" + std::to_string(thread_id) + "_" + std::to_string(sector_id);
    dest_node = module_assignment[dest_name];
#else
    dest_node = (sector_id * 6) + MFU0_DEST_ID;
#endif
    module_name_str = "mfu0_" + std::to_string(sector_id) + "_mop_axis_interface_" + std::to_string(thread_id);
    std::strcpy(module_name, module_name_str.c_str());
    mfu0_mop_axis_interface[sector_id] = new axis_master_fifo_adapter<mfu_mop, sc_bv<MFU_MOP_BITWIDTH>>(
        module_name, inst, mop_interface_dataw, 1, MFU_MOP_BITWIDTH, dest_node, MFU_INST_STREAM_ID);
    mfu0_mop_axis_interface[sector_id]->clk(clk);
    mfu0_mop_axis_interface[sector_id]->rst(rst);
    mfu0_mop_axis_interface[sector_id]->fifo_rdy(mfu0_mop_rdy_signal[sector_id]);
    mfu0_mop_axis_interface[sector_id]->fifo_ren(mfu0_mop_ren_signal[sector_id]);
    mfu0_mop_axis_interface[sector_id]->fifo_rdata(mfu0_mop_rdata_signal[sector_id]);
    mfu0_mop_axis_interface[sector_id]->axis_port.ConnectToPort(mfu0_mop_interface[sector_id]);

#ifdef RADSIM
    dest_name = "axis_vector_elementwise_" + std::to_string(thread_id) + "_" + std::to_string(sector_id);
    dest_node = module_assignment[dest_name];
#else
    dest_node = (sector_id * 6) + MFU1_DEST_ID;
#endif
    module_name_str = "mfu1_" + std::to_string(sector_id) + "_mop_axis_interface_" + std::to_string(thread_id);
    std::strcpy(module_name, module_name_str.c_str());
    mfu1_mop_axis_interface[sector_id] = new axis_master_fifo_adapter<mfu_mop, sc_bv<MFU_MOP_BITWIDTH>>(
        module_name, inst, mop_interface_dataw, 1, MFU_MOP_BITWIDTH, dest_node, MFU_INST_STREAM_ID + 1);
    mfu1_mop_axis_interface[sector_id]->clk(clk);
    mfu1_mop_axis_interface[sector_id]->rst(rst);
    mfu1_mop_axis_interface[sector_id]->fifo_rdy(mfu1_mop_rdy_signal[sector_id]);
    mfu1_mop_axis_interface[sector_id]->fifo_ren(mfu1_mop_ren_signal[sector_id]);
    mfu1_mop_axis_interface[sector_id]->fifo_rdata(mfu1_mop_rdata_signal[sector_id]);
    mfu1_mop_axis_interface[sector_id]->axis_port.ConnectToPort(mfu1_mop_interface[sector_id]);
  }

#ifdef RADSIM
  dest_name = "axis_ld_" + std::to_string(thread_id);
  dest_node = module_assignment[dest_name];
#else
  dest_node = ((SECTORS / 2) * 6) + LD_DEST_ID + 1;
#endif
  module_name_str = "ld_mop_axis_interface_" + std::to_string(thread_id);
  std::strcpy(module_name, module_name_str.c_str());
  ld_mop_axis_interface = new axis_master_fifo_adapter<ld_mop, sc_bv<LD_MOP_BITWIDTH>>(
      module_name, inst, mop_interface_dataw, 1, LD_MOP_BITWIDTH, dest_node, LD_INST_STREAM_ID);
  ld_mop_axis_interface->clk(clk);
  ld_mop_axis_interface->rst(rst);
  ld_mop_axis_interface->fifo_rdy(ld_mop_rdy_signal);
  ld_mop_axis_interface->fifo_ren(ld_mop_ren_signal);
  ld_mop_axis_interface->fifo_rdata(ld_mop_rdata_signal);
  ld_mop_axis_interface->axis_port.ConnectToPort(ld_mop_interface);
}

axis_inst_dispatch::~axis_inst_dispatch() {
  delete inst_dispatch_module;
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    delete sector_mop_axis_interface[sector_id];
    delete evrf_mop_axis_interface[sector_id];
    delete mfu0_mop_axis_interface[sector_id];
    delete mfu1_mop_axis_interface[sector_id];
  }
  delete ld_mop_axis_interface;
}
