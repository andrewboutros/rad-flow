#include <npu.hpp>

#define INST_AXIS_DATAW 512
#define WB0_AXIS_DATAW 512
#define WB1_AXIS_DATAW 512
#define FF_AXIS_DATAW 512

npu::npu(const sc_module_name& name)
    : sc_module(name),
      sectors_inst_signals("sectors_inst_signals"),
      evrf_inst_signals("evrf_inst_signals"),
      mfu0_inst_signals("mfu0_inst_signals"),
      mfu1_inst_signals("mfu1_inst_signals"),
      ld_inst_signals("ld_inst_signals"),
      sector_chain_fifo_rdy_signals("sector_chain_fifo_rdy_signals"),
      sector_chain_fifo_ren_signals("sector_chain_fifo_ren_signals"),
      sector_chain_fifo_rdata_signals("sector_chain_fifo_rdata_signals"),
      mrf_waddr("mrf_waddr"),
      mrf_wdata("mrf_wdata"),
      mrf_wid("mrf_wid"),
      ififo_rdy("ififo_rdy"),
      ififo_wen("ififo_ren"),
      ififo_wdata("ififo_wdata"),
      ofifo_rdy("ofifo_rdy"),
      ofifo_ren("ofifo_ren"),
      ofifo_rdata("ofifo_rdata") {
  init_vector<sc_out<bool>>::init_sc_vector(ififo_rdy, THREADS, CORES);
  init_vector<sc_in<bool>>::init_sc_vector(ififo_wen, THREADS, CORES);
  init_vector<sc_in<data_vector<tb_input_precision>>>::init_sc_vector(ififo_wdata, THREADS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(ofifo_rdy, THREADS, CORES);
  init_vector<sc_in<bool>>::init_sc_vector(ofifo_ren, THREADS, CORES);
  init_vector<sc_out<data_vector<tb_output_precision>>>::init_sc_vector(ofifo_rdata, THREADS, CORES);

  init_vector<axis_signal>::init_sc_vector(sectors_inst_signals, THREADS, SECTORS);
  init_vector<axis_signal>::init_sc_vector(evrf_inst_signals, THREADS, SECTORS);
  init_vector<axis_signal>::init_sc_vector(mfu0_inst_signals, THREADS, SECTORS);
  init_vector<axis_signal>::init_sc_vector(mfu1_inst_signals, THREADS, SECTORS);
  init_vector<axis_signal>::init_sc_vector(ld_inst_signals, THREADS);

  init_vector<axis_signal>::init_sc_vector(sector_to_evrf_signals, THREADS, SECTORS);
  init_vector<axis_signal>::init_sc_vector(evrf_to_mfu0_axis_signals, THREADS, SECTORS);
  init_vector<axis_signal>::init_sc_vector(mfu0_to_mfu1_axis_signals, THREADS, SECTORS);
  init_vector<axis_signal>::init_sc_vector(mfu1_to_ld_axis_signals, THREADS, SECTORS);
  init_vector<axis_signal>::init_sc_vector(wb_axis_signals, THREADS, SECTORS, NUM_PIPELINE_BLOCKS - 2);
  init_vector<axis_signal>::init_sc_vector(ld_wb0_axis_signals, THREADS);
  init_vector<axis_signal>::init_sc_vector(ld_wb1_axis_signals, THREADS, SECTORS);

  init_vector<sc_signal<bool>>::init_sc_vector(sector_chain_fifo_rdy_signals, SECTORS, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_chain_fifo_ren_signals, SECTORS, THREADS, CORES);
  init_vector<sc_signal<data_vector<tb_input_precision>>>::init_sc_vector(sector_chain_fifo_rdata_signals, SECTORS,
                                                                          THREADS, CORES);

  // Initialize & connect NPU blocks
  first_mvu_sector =
      new axis_mvu_sector("axis_mvu_sector_0", 0, INST_AXIS_DATAW, WB0_AXIS_DATAW, FF_AXIS_DATAW, WB0_AXIS_DATAW);
  first_mvu_sector->clk(clk);
  first_mvu_sector->rst(rst);
  first_mvu_sector->mrf_waddr(mrf_waddr);
  first_mvu_sector->mrf_wdata(mrf_wdata);
  first_mvu_sector->mrf_wid(mrf_wid);
  first_mvu_sector->sector_chain_ofifo_rdy(sector_chain_fifo_rdy_signals[0]);
  first_mvu_sector->sector_chain_ofifo_ren(sector_chain_fifo_ren_signals[0]);
  first_mvu_sector->sector_chain_ofifo_rdata(sector_chain_fifo_rdata_signals[0]);

  char module_name[NAME_LENGTH];
  std::string module_name_str;
  mvu_sectors.resize(SECTORS - 1);
  for (unsigned int sector_id = 1; sector_id < SECTORS; sector_id++) {
    module_name_str = "axis_mvu_sector_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    mvu_sectors[sector_id - 1] = new axis_mvu_sector_chain(module_name, sector_id, INST_AXIS_DATAW, WB0_AXIS_DATAW,
                                                           FF_AXIS_DATAW, WB0_AXIS_DATAW);
    mvu_sectors[sector_id - 1]->clk(clk);
    mvu_sectors[sector_id - 1]->rst(rst);
    mvu_sectors[sector_id - 1]->mrf_waddr(mrf_waddr);
    mvu_sectors[sector_id - 1]->mrf_wdata(mrf_wdata);
    mvu_sectors[sector_id - 1]->mrf_wid(mrf_wid);
    mvu_sectors[sector_id - 1]->sector_chain_ofifo_rdy(sector_chain_fifo_rdy_signals[sector_id]);
    mvu_sectors[sector_id - 1]->sector_chain_ofifo_ren(sector_chain_fifo_ren_signals[sector_id]);
    mvu_sectors[sector_id - 1]->sector_chain_ofifo_rdata(sector_chain_fifo_rdata_signals[sector_id]);
    mvu_sectors[sector_id - 1]->wb_fifo_rdy(sector_chain_fifo_rdy_signals[sector_id - 1]);
    mvu_sectors[sector_id - 1]->wb_fifo_ren(sector_chain_fifo_ren_signals[sector_id - 1]);
    mvu_sectors[sector_id - 1]->wb_fifo_rdata(sector_chain_fifo_rdata_signals[sector_id - 1]);
  }

  evrf_blocks.resize(THREADS);
  mfu0_blocks.resize(THREADS);
  mfu1_blocks.resize(THREADS);
  ld_blocks.resize(THREADS);
  wb_switches.resize(THREADS);
  inst_dispatcher.resize(THREADS);
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    evrf_blocks[thread_id].resize(SECTORS);
    mfu0_blocks[thread_id].resize(SECTORS);
    mfu1_blocks[thread_id].resize(SECTORS);
    wb_switches[thread_id].resize(SECTORS);

    module_name_str = "axis_inst_dispatcher_" + std::to_string(thread_id);
    std::strcpy(module_name, module_name_str.c_str());
    inst_dispatcher[thread_id] = new axis_inst_dispatch(module_name, INST_AXIS_DATAW, thread_id);
    inst_dispatcher[thread_id]->clk(clk);
    inst_dispatcher[thread_id]->rst(rst);
    inst_dispatcher[thread_id]->start_pc(start_pc);
    inst_dispatcher[thread_id]->end_pc(end_pc);
    inst_dispatcher[thread_id]->start(start);
    inst_dispatcher[thread_id]->inst_wdata(inst_wdata);
    inst_dispatcher[thread_id]->inst_waddr(inst_waddr);
    inst_dispatcher[thread_id]->inst_wen(inst_wen);

    module_name_str = "axis_ld_" + std::to_string(thread_id);
    std::strcpy(module_name, module_name_str.c_str());
    ld_blocks[thread_id] =
        new axis_loader(module_name, INST_AXIS_DATAW, WB0_AXIS_DATAW, WB1_AXIS_DATAW, FF_AXIS_DATAW, thread_id);
    ld_blocks[thread_id]->clk(clk);
    ld_blocks[thread_id]->rst(rst);
    ld_blocks[thread_id]->ext_input_fifo_rdy(ififo_rdy[thread_id]);
    ld_blocks[thread_id]->ext_input_fifo_wen(ififo_wen[thread_id]);
    ld_blocks[thread_id]->ext_input_fifo_wdata(ififo_wdata[thread_id]);
    ld_blocks[thread_id]->ext_output_fifo_rdy(ofifo_rdy[thread_id]);
    ld_blocks[thread_id]->ext_output_fifo_ren(ofifo_ren[thread_id]);
    ld_blocks[thread_id]->ext_output_fifo_rdata(ofifo_rdata[thread_id]);

    for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
      std::unordered_map<unsigned int, unsigned int> switch_dest_translation_table;
#ifdef RADSIM
      std::string dest_name = "axis_evrf_" + std::to_string(thread_id) + "_" + std::to_string(sector_id);
      switch_dest_translation_table.insert({{module_assignment[dest_name], 0}});
      dest_name = "axis_mfu0_" + std::to_string(thread_id) + "_" + std::to_string(sector_id);
      switch_dest_translation_table.insert({{module_assignment[dest_name], 1}});
      dest_name = "axis_mfu1_" + std::to_string(thread_id) + "_" + std::to_string(sector_id);
      switch_dest_translation_table.insert({{module_assignment[dest_name], 2}});
#else
      switch_dest_translation_table.insert({{1, 0}});
      switch_dest_translation_table.insert({{2, 1}});
      switch_dest_translation_table.insert({{3, 2}});
#endif

      module_name_str = "wb_switch_" + std::to_string(sector_id) + "_" + std::to_string(thread_id);
      std::strcpy(module_name, module_name_str.c_str());
      wb_switches[thread_id][sector_id] =
          new axis_switch(module_name, NUM_PIPELINE_BLOCKS - 2, switch_dest_translation_table);
      wb_switches[thread_id][sector_id]->clk(clk);
      wb_switches[thread_id][sector_id]->rst(rst);

      module_name_str = "axis_evrf_" + std::to_string(sector_id) + "_" + std::to_string(thread_id);
      std::strcpy(module_name, module_name_str.c_str());
      evrf_blocks[thread_id][sector_id] = new axis_evrf(module_name, sector_id, thread_id, INST_AXIS_DATAW,
                                                        WB1_AXIS_DATAW, FF_AXIS_DATAW, FF_AXIS_DATAW);
      evrf_blocks[thread_id][sector_id]->clk(clk);
      evrf_blocks[thread_id][sector_id]->rst(rst);

      module_name_str = "axis_mfu0_" + std::to_string(sector_id) + "_" + std::to_string(thread_id);
      std::strcpy(module_name, module_name_str.c_str());
      mfu0_blocks[thread_id][sector_id] = new axis_mfu(module_name, 0, sector_id, thread_id, INST_AXIS_DATAW,
                                                       WB1_AXIS_DATAW, FF_AXIS_DATAW, FF_AXIS_DATAW);
      mfu0_blocks[thread_id][sector_id]->clk(clk);
      mfu0_blocks[thread_id][sector_id]->rst(rst);

      module_name_str = "axis_mfu1_" + std::to_string(sector_id) + "_" + std::to_string(thread_id);
      std::strcpy(module_name, module_name_str.c_str());
      mfu1_blocks[thread_id][sector_id] = new axis_mfu(module_name, 1, sector_id, thread_id, INST_AXIS_DATAW,
                                                       WB1_AXIS_DATAW, FF_AXIS_DATAW, FF_AXIS_DATAW);
      mfu1_blocks[thread_id][sector_id]->clk(clk);
      mfu1_blocks[thread_id][sector_id]->rst(rst);

      if (sector_id == 0)
        sectors_inst_signals[thread_id][sector_id].Connect(inst_dispatcher[thread_id]->sector_mop_interface[sector_id],
                                                           first_mvu_sector->sector_inst_interface[thread_id]);
      else
        sectors_inst_signals[thread_id][sector_id].Connect(inst_dispatcher[thread_id]->sector_mop_interface[sector_id],
                                                           mvu_sectors[sector_id - 1]->sector_inst_interface[thread_id]);
      evrf_inst_signals[thread_id][sector_id].Connect(inst_dispatcher[thread_id]->evrf_mop_interface[sector_id],
                                                      evrf_blocks[thread_id][sector_id]->evrf_inst_interface);
      mfu0_inst_signals[thread_id][sector_id].Connect(inst_dispatcher[thread_id]->mfu0_mop_interface[sector_id],
                                                      mfu0_blocks[thread_id][sector_id]->mfu_inst_interface);
      mfu1_inst_signals[thread_id][sector_id].Connect(inst_dispatcher[thread_id]->mfu1_mop_interface[sector_id],
                                                      mfu1_blocks[thread_id][sector_id]->mfu_inst_interface);

      if (sector_id == 0)
        sector_to_evrf_signals[thread_id][sector_id].Connect(first_mvu_sector->sector_ofifo_interface[thread_id],
                                                             evrf_blocks[thread_id][sector_id]->evrf_ififo_interface);
      else
        sector_to_evrf_signals[thread_id][sector_id].Connect(mvu_sectors[sector_id - 1]->sector_ofifo_interface[thread_id],
                                                             evrf_blocks[thread_id][sector_id]->evrf_ififo_interface);

      evrf_to_mfu0_axis_signals[thread_id][sector_id].Connect(evrf_blocks[thread_id][sector_id]->evrf_ofifo_interface,
                                                              mfu0_blocks[thread_id][sector_id]->mfu_ififo_interface);
      mfu0_to_mfu1_axis_signals[thread_id][sector_id].Connect(mfu0_blocks[thread_id][sector_id]->mfu_ofifo_interface,
                                                              mfu1_blocks[thread_id][sector_id]->mfu_ififo_interface);
      mfu1_to_ld_axis_signals[thread_id][sector_id].Connect(
          mfu1_blocks[thread_id][sector_id]->mfu_ofifo_interface,
          ld_blocks[thread_id]->loader_input_fifo_interfaces[sector_id]);

      ld_wb1_axis_signals[thread_id][sector_id].Connect(ld_blocks[thread_id]->loader_wb1_interfaces[sector_id],
                                                        wb_switches[thread_id][sector_id]->switch_slave_interface);

      wb_axis_signals[thread_id][sector_id][0].Connect(wb_switches[thread_id][sector_id]->switch_master_interfaces[0],
                                                       evrf_blocks[thread_id][sector_id]->evrf_wb_interface);
      wb_axis_signals[thread_id][sector_id][1].Connect(wb_switches[thread_id][sector_id]->switch_master_interfaces[1],
                                                       mfu0_blocks[thread_id][sector_id]->mfu_wb_interface);
      wb_axis_signals[thread_id][sector_id][2].Connect(wb_switches[thread_id][sector_id]->switch_master_interfaces[2],
                                                       mfu1_blocks[thread_id][sector_id]->mfu_wb_interface);
    }

    ld_inst_signals[thread_id].Connect(inst_dispatcher[thread_id]->ld_mop_interface,
                                       ld_blocks[thread_id]->loader_inst_interface);
    ld_wb0_axis_signals[thread_id].Connect(ld_blocks[thread_id]->loader_wb0_interface,
                                           first_mvu_sector->sector_wb_interface[thread_id]);
  }
}

npu::~npu() {
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    if (sector_id == 0)
      delete first_mvu_sector;
    else
      delete mvu_sectors[sector_id - 1];
    for (unsigned int thread_id = 0; thread_id < SECTORS; thread_id++) {
      delete evrf_blocks[thread_id][sector_id];
      delete mfu0_blocks[thread_id][sector_id];
      delete mfu1_blocks[thread_id][sector_id];
      delete wb_switches[thread_id][sector_id];
    }
  }
  for (unsigned int thread_id = 0; thread_id < SECTORS; thread_id++) {
    delete ld_blocks[thread_id];
    delete inst_dispatcher[thread_id];
  }
}