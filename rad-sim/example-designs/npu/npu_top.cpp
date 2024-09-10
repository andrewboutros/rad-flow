#include <npu_top.hpp>

npu_top::npu_top(const sc_module_name &name, RADSimDesignContext* radsim_design) : RADSimDesignTop(radsim_design),
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
  init_vector<sc_signal<bool>>::init_sc_vector(sector_chain_fifo_rdy_signals, SECTORS, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_chain_fifo_ren_signals, SECTORS, THREADS, CORES);
  init_vector<sc_signal<data_vector<tb_input_precision>>>::init_sc_vector(sector_chain_fifo_rdata_signals, SECTORS,
                                                                          THREADS, CORES);

  mvu_sectors.resize(SECTORS - 1);
  vector_elementwise_blocks.resize(THREADS);
  inst_dispatcher.resize(THREADS);
  char module_name[NAME_LENGTH];
  std::string module_name_str;

  first_mvu_sector = new axis_mvu_sector("axis_mvu_sector_0", 0, radsim_design);
  first_mvu_sector->rst(rst);
  first_mvu_sector->mrf_waddr(mrf_waddr);
  first_mvu_sector->mrf_wdata(mrf_wdata);
  first_mvu_sector->mrf_wid(mrf_wid);
  first_mvu_sector->sector_chain_ofifo_rdy(sector_chain_fifo_rdy_signals[0]);
  first_mvu_sector->sector_chain_ofifo_ren(sector_chain_fifo_ren_signals[0]);
  first_mvu_sector->sector_chain_ofifo_rdata(sector_chain_fifo_rdata_signals[0]);

  for (unsigned int sector_id = 1; sector_id < SECTORS; sector_id++) {
    module_name_str = "axis_mvu_sector_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    mvu_sectors[sector_id - 1] = new axis_mvu_sector_chain(module_name, sector_id, radsim_design);
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

  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    module_name_str = "axis_inst_dispatcher_" + std::to_string(thread_id);
    std::strcpy(module_name, module_name_str.c_str());
    inst_dispatcher[thread_id] = new axis_inst_dispatch(module_name, thread_id, radsim_design);
    inst_dispatcher[thread_id]->rst(rst);
    inst_dispatcher[thread_id]->start_pc(start_pc);
    inst_dispatcher[thread_id]->end_pc(end_pc);
    inst_dispatcher[thread_id]->start(start);
    inst_dispatcher[thread_id]->inst_wdata(inst_wdata);
    inst_dispatcher[thread_id]->inst_waddr(inst_waddr);
    inst_dispatcher[thread_id]->inst_wen(inst_wen);

    module_name_str = "axis_vector_elementwise_" + std::to_string(thread_id);
    std::strcpy(module_name, module_name_str.c_str());
    vector_elementwise_blocks[thread_id] = new axis_vector_elementwise(module_name, thread_id, radsim_design);
    vector_elementwise_blocks[thread_id]->rst(rst);
    vector_elementwise_blocks[thread_id]->ext_input_fifo_rdy(ififo_rdy[thread_id]);
    vector_elementwise_blocks[thread_id]->ext_input_fifo_wen(ififo_wen[thread_id]);
    vector_elementwise_blocks[thread_id]->ext_input_fifo_wdata(ififo_wdata[thread_id]);
    vector_elementwise_blocks[thread_id]->ext_output_fifo_rdy(ofifo_rdy[thread_id]);
    vector_elementwise_blocks[thread_id]->ext_output_fifo_ren(ofifo_ren[thread_id]);
    vector_elementwise_blocks[thread_id]->ext_output_fifo_rdata(ofifo_rdata[thread_id]);
  }                                                             

  this->portal_inst->rst(rst);
  radsim_design->BuildDesignContext("npu.place", "npu.clks");
  radsim_design->CreateSystemNoCs(rst);
  radsim_design->ConnectModulesToNoC();
}

npu_top::~npu_top() {
  delete first_mvu_sector;
  for (unsigned int sector_id = 1; sector_id < SECTORS; sector_id++) delete mvu_sectors[sector_id - 1];
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    delete vector_elementwise_blocks[thread_id];
    delete inst_dispatcher[thread_id];
  }
}