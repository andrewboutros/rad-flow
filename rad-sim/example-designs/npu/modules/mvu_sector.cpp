#include "mvu_sector.hpp"

mvu_sector::mvu_sector(const sc_module_name& name, unsigned int id)
    : sc_module(name),
      inst_valid_pipeline("inst_valid_pipeline", SECTOR_INST_TO_DPES_PIPELINE),
      inst_pipeline("inst_pipeline", SECTOR_INST_PIPELINE),
      mrf_raddr_pipeline("mrf_raddr_pipeline", SECTOR_MRF_ADDR_PIPELINE),
      vrf_rdata_pipeline("vrf_rdata_pipeline"),
      dpe_result_signal("dpe_result_signal"),
      dpe_result_valid_signal("dpe_result_valid_signal"),
      dpe_valid_a_signal("dpe_valid_a_signal"), 
      dpe_valid_b_signal("dpe_valid_b_signal"), 
      dpe_reg_use_sel_signal("dpe_reg_use_sel_signal"), 
      dpe_reg_shift_sel_signal("dpe_reg_shift_sel_signal"),
      reduction_pipeline("reduction_pipeline"),
      reduction_valid_pipeline("reduction_valid_pipeline",
                               SECTOR_REDUCTION_PIPELINE + SECTOR_REDUCTION_TO_ACCUM_PIPELINE),
      accum_size_signal("accum_size_signal"),
      accum_op_signal("accum_op_signal"),
      accum_to_ofifo_pipeline("accum_to_ofifo_pipeline"),
      accum_to_ofifo_valid_pipeline("accum_to_ofifo_valid_pipeline", SECTOR_ACCUM_TO_OFIFO_PIPELINE),
      inst_fifo_full("inst_fifo_full", THREADS),
      inst_fifo_almost_full("inst_fifo_almost_full", THREADS),
      inst_fifo_empty("inst_fifo_empty", THREADS),
      inst_fifo_almost_empty("inst_fifo_almost_empty", THREADS),
      inst_fifo_ren("inst_fifo_ren", THREADS),
      inst_fifo_rdata("inst_fifo_rdata", THREADS),
      ofifo_full("ofifo_full"),
      ofifo_almost_full("ofifo_almost_full"),
      ofifo_empty("ofifo_empty"),
      ofifo_almost_empty("ofifo_almost_empty"),
      ofifo_wen_signal("ofifo_wen_signal"),
      wb_ififo_full("wb_ififo_full"),
      wb_ififo_almost_full("wb_ififo_almost_full"),
      wb_ififo_empty("wb_ififo_empty"),
      wb_ififo_almost_empty("wb_ififo_almost_empty"),
      wb_ififo_wen("wb_ififo_wen"),
      wb_ififo_ren("wb_ififo_ren"),
      wb_ififo_rdata("wb_ififo_rdata"),
      sector_chain_ofifo_full("sector_chain_ofifo_full"),
      sector_chain_ofifo_almost_full("sector_chain_ofifo_almost_full"),
      sector_chain_ofifo_empty("sector_chain_ofifo_empty"),
      sector_chain_ofifo_almost_empty("sector_chain_ofifo_almost_empty"),
      sector_chain_ofifo_wen("sector_chain_ofifo_wen"),
      sector_chain_ofifo_wen_signal("sector_chain_ofifo_wen_signal"),
      mrf_waddr_signal("mrf_waddr_signal"),
      mrf_wen_signal("mrf_wen_signal"),
      mrf_rdata_signal("mrf_rdata_signal"),
      vrf_raddr_signal("vrf_raddr_signal"),
      vrf_rd_id_signal("vrf_rd_id_signal"),
      vrf_waddr_signal("vrf_waddr_signal"),
      vrf_wdata_signal("vrf_wdata_signal"),
      vrf_wen_signal("vrf_wen_signal"),
      vrf_rdata_signal("vrf_rdata_signal"),
      current_tag("current_tag", THREADS),
      sector_stall("sector_stall"),
      active_thread("active_thread"),
      thread_id_pipeline("thread_id_pipeline", SECTOR_INST_PIPELINE),
      thread_locked("thread_locked"),
      inst("inst", THREADS),
      inst_fifo_wen("inst_fifo_wen", THREADS),
      inst_fifo_rdy("inst_fifo_rdy", THREADS),
      mrf_waddr("mrf_waddr"),
      mrf_wdata("mrf_wdata"),
      mrf_wid("mrf_wid"),
      wb_fifo_rdy("wb_fifo_rdy"),
      wb_fifo_ren("wb_fifo_ren"),
      wb_fifo_rdata("wb_fifo_rdata"),
      sector_chain_ofifo_rdy("sector_chain_ofifo_rdy"),
      sector_chain_ofifo_ren("sector_chain_ofifo_ren"),
      sector_chain_ofifo_rdata("sector_chain_ofifo_rdata"),
      sector_ofifo_rdy("sector_ofifo_rdy"),
      sector_ofifo_ren("sector_ofifo_ren"),
      sector_ofifo_rdata("sector_ofifo_rdata") {

  sector_id = id;

  init_vector<sc_signal<data_vector<tb_input_precision>>>::init_sc_vector(vrf_rdata_pipeline, CORES, TILES,
                                                                          SECTOR_VRF_DATA_PIPELINE);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(dpe_result_signal, CORES, DPES_PER_SECTOR,
                                                                           TILES);
  init_vector<sc_signal<bool>>::init_sc_vector(dpe_result_valid_signal, CORES, DPES_PER_SECTOR, TILES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(
      reduction_pipeline, SECTOR_REDUCTION_PIPELINE + SECTOR_REDUCTION_TO_ACCUM_PIPELINE, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(accum_to_ofifo_pipeline,
                                                                           SECTOR_ACCUM_TO_OFIFO_PIPELINE, CORES);

  init_vector<sc_in<bool>>::init_sc_vector(wb_fifo_rdy, THREADS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(wb_fifo_ren, THREADS, CORES);
  init_vector<sc_in<data_vector<tb_input_precision>>>::init_sc_vector(wb_fifo_rdata, THREADS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(sector_chain_ofifo_rdy, THREADS, CORES);
  init_vector<sc_in<bool>>::init_sc_vector(sector_chain_ofifo_ren, THREADS, CORES);
  init_vector<sc_out<data_vector<tb_input_precision>>>::init_sc_vector(sector_chain_ofifo_rdata, THREADS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(sector_ofifo_rdy, THREADS, CORES);
  init_vector<sc_in<bool>>::init_sc_vector(sector_ofifo_ren, THREADS, CORES);
  init_vector<sc_out<data_vector<tb_output_precision>>>::init_sc_vector(sector_ofifo_rdata, THREADS, CORES);

  init_vector<sc_signal<bool>>::init_sc_vector(ofifo_full, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ofifo_empty, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ofifo_almost_full, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ofifo_almost_empty, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ofifo_wen_signal, THREADS);

  init_vector<sc_signal<bool>>::init_sc_vector(wb_ififo_full, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(wb_ififo_empty, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(wb_ififo_almost_full, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(wb_ififo_almost_empty, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(wb_ififo_wen, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(wb_ififo_ren, THREADS, CORES);
  init_vector<sc_signal<data_vector<tb_input_precision>>>::init_sc_vector(wb_ififo_rdata, THREADS, CORES);

  init_vector<sc_signal<bool>>::init_sc_vector(sector_chain_ofifo_full, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_chain_ofifo_empty, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_chain_ofifo_almost_full, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_chain_ofifo_almost_empty, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_chain_ofifo_wen, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_chain_ofifo_wen_signal, THREADS);

  init_vector<sc_signal<bool>>::init_sc_vector(mrf_wen_signal, DPES_PER_SECTOR, TILES);
  init_vector<sc_signal<data_vector<tb_input_precision>>>::init_sc_vector(mrf_rdata_signal, DPES_PER_SECTOR, TILES);
  init_vector<sc_signal<unsigned int>>::init_sc_vector(vrf_waddr_signal, THREADS, CORES, TILES);
  init_vector<sc_signal<data_vector<tb_input_precision>>>::init_sc_vector(vrf_wdata_signal, THREADS, CORES, TILES);
  init_vector<sc_signal<bool>>::init_sc_vector(vrf_wen_signal, THREADS, CORES, TILES);
  init_vector<sc_signal<data_vector<tb_input_precision>>>::init_sc_vector(vrf_rdata_signal, THREADS, CORES, TILES);

  // Create MVU tile instruction FIFO
  inst_fifo.resize(THREADS);
  char inst_fifo_name[NAME_LENGTH];
  std::string inst_fifo_name_str;
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    inst_fifo_name_str = "sector" + std::to_string(sector_id) + "_inst_fifo_" + std::to_string(thread_id);
    std::strcpy(inst_fifo_name, inst_fifo_name_str.c_str());
    inst_fifo[thread_id] = new instruction_fifo<mvu_uop>(inst_fifo_name, UOP_FIFO_DEPTH, UOP_FIFO_ALMOST_FULL_DEPTH, 0);
    inst_fifo[thread_id]->clk(clk);
    inst_fifo[thread_id]->rst(rst);
    inst_fifo[thread_id]->wen(inst_fifo_wen[thread_id]);
    inst_fifo[thread_id]->ren(inst_fifo_ren[thread_id]);
    inst_fifo[thread_id]->wdata(inst[thread_id]);
    inst_fifo[thread_id]->full(inst_fifo_full[thread_id]);
    inst_fifo[thread_id]->almost_full(inst_fifo_almost_full[thread_id]);
    inst_fifo[thread_id]->empty(inst_fifo_empty[thread_id]);
    inst_fifo[thread_id]->almost_empty(inst_fifo_almost_empty[thread_id]);
    inst_fifo[thread_id]->rdata(inst_fifo_rdata[thread_id]);
  }

  // Create DPES and MRFs
  dpes.resize(CORES);
  char dpe_name[NAME_LENGTH], mrf_name[NAME_LENGTH];
  std::string dpe_name_str, mrf_name_str;
  for (unsigned int core_id = 0; core_id < CORES; core_id++) {
    dpes[core_id].resize(DPES_PER_SECTOR);
    for (unsigned int dpe_id = 0; dpe_id < DPES_PER_SECTOR; dpe_id++) {
      dpes[core_id][dpe_id].resize(TILES);
      for (unsigned int tile_id = 0; tile_id < TILES; tile_id++) {
        dpe_name_str = "dpe" + std::to_string(core_id) + "_" + std::to_string(dpe_id) + "_" + std::to_string(tile_id);
        std::strcpy(dpe_name, dpe_name_str.c_str());
        dpes[core_id][dpe_id][tile_id] = new dpe(
            dpe_name, (tile_id * DPES_PER_SECTOR * SECTORS) + (sector_id * DPES_PER_SECTOR) + dpe_id, tile_id);
        dpes[core_id][dpe_id][tile_id]->clk(this->clk);
        dpes[core_id][dpe_id][tile_id]->rst(this->rst);
        dpes[core_id][dpe_id][tile_id]->vector_a(mrf_rdata_signal[dpe_id][tile_id]);
        dpes[core_id][dpe_id][tile_id]->valid_a(dpe_valid_a_signal);
        dpes[core_id][dpe_id][tile_id]->vector_b(vrf_rdata_pipeline[core_id][tile_id][SECTOR_VRF_DATA_PIPELINE - 1]);
        dpes[core_id][dpe_id][tile_id]->valid_b(dpe_valid_b_signal);
        dpes[core_id][dpe_id][tile_id]->reg_shift_sel(dpe_reg_shift_sel_signal);
        dpes[core_id][dpe_id][tile_id]->reg_use_sel(dpe_reg_use_sel_signal);
        dpes[core_id][dpe_id][tile_id]->result(dpe_result_signal[core_id][dpe_id][tile_id]);
        dpes[core_id][dpe_id][tile_id]->valid_result(dpe_result_valid_signal[core_id][dpe_id][tile_id]);
        dpes[core_id][dpe_id][tile_id]->stall(sector_stall);
      }
    }
  }

  mrfs.resize(DPES_PER_SECTOR);
  std::string mrf_filename, mrf_path;
  std::string npu_dir = radsim_config.GetStringKnob("radsim_user_design_root_dir");
  for (unsigned int dpe_id = 0; dpe_id < DPES_PER_SECTOR; dpe_id++) {
    mrfs[dpe_id].resize(TILES);
    for (unsigned int tile_id = 0; tile_id < TILES; tile_id++) {
      mrf_name_str = "mrf_" + std::to_string(tile_id) + "_" + std::to_string((sector_id * DPES_PER_SECTOR) + dpe_id);
      std::strcpy(mrf_name, mrf_name_str.c_str());
      mrf_filename = "/register_files/mrf_tile_" + std::to_string(tile_id) + "_dpe_" +
                     std::to_string((sector_id * DPES_PER_SECTOR) + dpe_id) + ".txt";
      mrf_path = npu_dir + mrf_filename;
      mrfs[dpe_id][tile_id] = new register_file<tb_input_precision>(
          mrf_name, (tile_id * DPES_PER_SECTOR * SECTORS) + (DPES_PER_SECTOR * sector_id) + dpe_id, MRF_DEPTH, LANES,
          mrf_path);
      mrfs[dpe_id][tile_id]->clk(clk);
      mrfs[dpe_id][tile_id]->rst(rst);
      mrfs[dpe_id][tile_id]->raddr(mrf_raddr_pipeline[SECTOR_MRF_ADDR_PIPELINE - 1]);
      mrfs[dpe_id][tile_id]->wdata(mrf_wdata);
      mrfs[dpe_id][tile_id]->waddr(mrf_waddr_signal);
      mrfs[dpe_id][tile_id]->wen(mrf_wen_signal[dpe_id][tile_id]);
      mrfs[dpe_id][tile_id]->rdata(mrf_rdata_signal[dpe_id][tile_id]);
      mrfs[dpe_id][tile_id]->clk_en(sector_stall);
    }
  }

  // Create VRFs
  vrfs.resize(THREADS);
  char vrf_name[NAME_LENGTH];
  std::string vrf_name_str;
  std::string vrf_filename;
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    vrfs[thread_id].resize(CORES);
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      vrfs[thread_id][core_id].resize(TILES);
      for (unsigned int tile_id = 0; tile_id < TILES; tile_id++) {
        vrf_name_str = "mvu-vrf_" + std::to_string(tile_id) + "_" + std::to_string(core_id) + "_" +
                       std::to_string(sector_id) + "_" + std::to_string(thread_id);
        std::strcpy(vrf_name, vrf_name_str.c_str());
        vrfs[thread_id][core_id][tile_id] = new piso_register_file<tb_input_precision>(
            vrf_name, (sector_id * TILES * CORES) + (tile_id * CORES) + core_id, VRF_DEPTH, LANES, vrf_filename);
        vrfs[thread_id][core_id][tile_id]->clk(clk);
        vrfs[thread_id][core_id][tile_id]->rst(rst);
        vrfs[thread_id][core_id][tile_id]->raddr(vrf_raddr_signal);
        vrfs[thread_id][core_id][tile_id]->rd_id(vrf_rd_id_signal);
        vrfs[thread_id][core_id][tile_id]->wdata(vrf_wdata_signal[thread_id][core_id][tile_id]);
        vrfs[thread_id][core_id][tile_id]->waddr(vrf_waddr_signal[thread_id][core_id][tile_id]);
        vrfs[thread_id][core_id][tile_id]->wen(vrf_wen_signal[thread_id][core_id][tile_id]);
        vrfs[thread_id][core_id][tile_id]->rdata(vrf_rdata_signal[thread_id][core_id][tile_id]);
        vrfs[thread_id][core_id][tile_id]->clk_en(sector_stall);
      }
    }
  }

  // Create accumulator
  accumulator = new accum("accum");
  accumulator->clk(clk);
  accumulator->rst(rst);
  accumulator->valid_in(reduction_valid_pipeline[SECTOR_REDUCTION_PIPELINE + SECTOR_REDUCTION_TO_ACCUM_PIPELINE - 1]);
  accumulator->accum_size(accum_size_signal);
  accumulator->accum_op(accum_op_signal);
  accumulator->accum_in(reduction_pipeline[SECTOR_REDUCTION_PIPELINE + SECTOR_REDUCTION_TO_ACCUM_PIPELINE - 1]);
  accumulator->valid_out(accum_to_ofifo_valid_pipeline[0]);
  accumulator->accum_out(accum_to_ofifo_pipeline[0]);
  accumulator->clk_en(sector_stall);

  // Create output FIFOs
  wb_ififos.resize(THREADS);
  sector_chain_ofifos.resize(THREADS);
  ofifos.resize(THREADS);
  std::string sector_chain_ofifo_name_str, wb_ififo_name_str, ofifo_name_str;
  char sector_chain_ofifo_name[NAME_LENGTH], wb_ififo_name[NAME_LENGTH], ofifo_name[NAME_LENGTH];
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    thread_order.push(thread_id);
    wb_ififos[thread_id].resize(CORES);
    sector_chain_ofifos[thread_id].resize(CORES);
    ofifos[thread_id].resize(CORES);
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      sector_chain_ofifo_name_str = "sector" + std::to_string(sector_id) + "_chain_ofifo_" + std::to_string(core_id) +
                                    "_t" + std::to_string(thread_id);
      std::strcpy(sector_chain_ofifo_name, sector_chain_ofifo_name_str.c_str());
      sector_chain_ofifos[thread_id][core_id] = new fifo<tb_input_precision>(
          sector_chain_ofifo_name, DATA_FIFO_DEPTH, LANES + 4, DATA_FIFO_DEPTH - SECTOR_INST_PIPELINE, 0);
      sector_chain_ofifos[thread_id][core_id]->clk(clk);
      sector_chain_ofifos[thread_id][core_id]->rst(rst);
      sector_chain_ofifos[thread_id][core_id]->wen(sector_chain_ofifo_wen[thread_id][core_id]);
      sector_chain_ofifos[thread_id][core_id]->ren(sector_chain_ofifo_ren[thread_id][core_id]);
      sector_chain_ofifos[thread_id][core_id]->wdata(wb_ififo_rdata[thread_id][core_id]);
      sector_chain_ofifos[thread_id][core_id]->full(sector_chain_ofifo_full[thread_id][core_id]);
      sector_chain_ofifos[thread_id][core_id]->almost_full(sector_chain_ofifo_almost_full[thread_id][core_id]);
      sector_chain_ofifos[thread_id][core_id]->empty(sector_chain_ofifo_empty[thread_id][core_id]);
      sector_chain_ofifos[thread_id][core_id]->almost_empty(sector_chain_ofifo_almost_empty[thread_id][core_id]);
      sector_chain_ofifos[thread_id][core_id]->rdata(sector_chain_ofifo_rdata[thread_id][core_id]);

      wb_ififo_name_str = "sector" + std::to_string(sector_id) + "_wb_ififo_" + std::to_string(core_id) + "_t" +
                          std::to_string(thread_id);
      std::strcpy(wb_ififo_name, wb_ififo_name_str.c_str());
      wb_ififos[thread_id][core_id] = new fifo<tb_input_precision>(wb_ififo_name, DATA_FIFO_DEPTH, LANES + 4,
                                                                   DATA_FIFO_DEPTH - SECTOR_INST_PIPELINE, 0);
      wb_ififos[thread_id][core_id]->clk(clk);
      wb_ififos[thread_id][core_id]->rst(rst);
      wb_ififos[thread_id][core_id]->wen(wb_ififo_wen[thread_id][core_id]);
      wb_ififos[thread_id][core_id]->ren(wb_ififo_ren[thread_id][core_id]);
      wb_ififos[thread_id][core_id]->wdata(wb_fifo_rdata[thread_id][core_id]);
      wb_ififos[thread_id][core_id]->full(wb_ififo_full[thread_id][core_id]);
      wb_ififos[thread_id][core_id]->almost_full(wb_ififo_almost_full[thread_id][core_id]);
      wb_ififos[thread_id][core_id]->empty(wb_ififo_empty[thread_id][core_id]);
      wb_ififos[thread_id][core_id]->almost_empty(wb_ififo_almost_empty[thread_id][core_id]);
      wb_ififos[thread_id][core_id]->rdata(wb_ififo_rdata[thread_id][core_id]);

      ofifo_name_str =
          "sector" + std::to_string(sector_id) + "_ofifo_" + std::to_string(core_id) + "_t" + std::to_string(thread_id);
      std::strcpy(ofifo_name, ofifo_name_str.c_str());
      ofifos[thread_id][core_id] =
          new asymmetric_fifo<tb_output_precision>(ofifo_name, DATA_FIFO_DEPTH, TB_NUM_DOTS * DPES_PER_SECTOR,
                                                   DPES_PER_SECTOR, DATA_FIFO_DEPTH - SECTOR_INST_PIPELINE, 0);
      ofifos[thread_id][core_id]->clk(clk);
      ofifos[thread_id][core_id]->rst(rst);
      ofifos[thread_id][core_id]->wen(ofifo_wen_signal[thread_id]);
      ofifos[thread_id][core_id]->ren(sector_ofifo_ren[thread_id][core_id]);
      ofifos[thread_id][core_id]->wdata(accum_to_ofifo_pipeline[SECTOR_ACCUM_TO_OFIFO_PIPELINE - 1][core_id]);
      ofifos[thread_id][core_id]->full(ofifo_full[thread_id][core_id]);
      ofifos[thread_id][core_id]->almost_full(ofifo_almost_full[thread_id][core_id]);
      ofifos[thread_id][core_id]->empty(ofifo_empty[thread_id][core_id]);
      ofifos[thread_id][core_id]->almost_empty(ofifo_almost_empty[thread_id][core_id]);
      ofifos[thread_id][core_id]->rdata(sector_ofifo_rdata[thread_id][core_id]);
    }
  }

  // Set sensitivity list for SC_METHOD & clock and reset signal for SC_CTHREAD
  SC_METHOD(Assign);
  sensitive << rst << inst_valid_pipeline[SECTOR_DISTRIBUTION_PIPELINE + SECTOR_MRF_DATA_PIPELINE - 1]
            << inst_pipeline[SECTOR_DISTRIBUTION_PIPELINE + SECTOR_MRF_DATA_PIPELINE - 1]
            << inst_pipeline[SECTOR_INST_TO_DPES_PIPELINE - 1] << mrf_waddr << mrf_wid
            << inst_valid_pipeline[SECTOR_INST_TO_DPES_PIPELINE - 1] << inst_pipeline[SECTOR_INST_TO_ACCUM_PIPELINE - 1]
            << accum_to_ofifo_valid_pipeline[SECTOR_ACCUM_TO_OFIFO_PIPELINE - 1]
            << thread_id_pipeline[SECTOR_INST_PIPELINE - 2] << active_thread;
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    sensitive << inst_fifo_empty[thread_id] << current_tag[thread_id] << inst_fifo_rdata[thread_id]
              << inst_fifo_almost_full[thread_id] << inst_fifo_ren[thread_id] << inst_fifo_wen[thread_id];
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      sensitive << wb_fifo_rdy[thread_id][core_id] << wb_fifo_rdata[thread_id][core_id]
                << ofifo_empty[thread_id][core_id] << ofifo_almost_full[thread_id][core_id]
                << sector_chain_ofifo_empty[thread_id][core_id] << sector_chain_ofifo_almost_full[thread_id][core_id]
                << wb_fifo_ren[thread_id][core_id];
      sensitive << wb_ififo_almost_full[thread_id][core_id] << wb_ififo_empty[thread_id][core_id]
                << wb_ififo_rdata[thread_id][core_id] << wb_ififo_ren[thread_id][core_id];
    }
  }
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

mvu_sector::~mvu_sector() {
  for (unsigned int core_id = 0; core_id < CORES; core_id++) {
    for (unsigned int tile_id = 0; tile_id < TILES; tile_id++) {
      for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) delete vrfs[thread_id][core_id][tile_id];
      for (unsigned int dpe_id = 0; dpe_id < DPES_PER_SECTOR; dpe_id++) delete dpes[core_id][dpe_id][tile_id];
    }
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
      delete ofifos[thread_id][core_id];
      delete sector_chain_ofifos[thread_id][core_id];
      delete wb_ififos[thread_id][core_id];
    }
  }
  for (unsigned int dpe_id = 0; dpe_id < DPES_PER_SECTOR; dpe_id++) {
    for (unsigned int tile_id = 0; tile_id < TILES; tile_id++) delete mrfs[dpe_id][tile_id];
  }
  delete accumulator;
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) delete inst_fifo[thread_id];
}

void mvu_sector::Tick() {
  // Reset logic
  for (unsigned int stage_id = 0; stage_id < SECTOR_MRF_ADDR_PIPELINE; stage_id++)
    mrf_raddr_pipeline[stage_id].write(0);
  for (unsigned int stage_id = 0; stage_id < SECTOR_INST_TO_DPES_PIPELINE; stage_id++)
    inst_valid_pipeline[stage_id].write(false);
  for (unsigned int stage_id = 0; stage_id < SECTOR_REDUCTION_PIPELINE + SECTOR_REDUCTION_TO_ACCUM_PIPELINE; stage_id++)
    reduction_valid_pipeline[stage_id].write(false);
  for (unsigned int stage_id = 1; stage_id < SECTOR_ACCUM_TO_OFIFO_PIPELINE; stage_id++)
    accum_to_ofifo_valid_pipeline[stage_id].write(false);
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) current_tag[thread_id].write(0);
  active_thread.write(0);
  thread_locked.write(true);
  for (unsigned int stage_id = 0; stage_id < SECTOR_INST_PIPELINE; stage_id++) thread_id_pipeline[stage_id].write(0);
  wait();

  // Sequential logic
  while (true) {
    // Update current tag value for hazard detection and resolution
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
      if (!wb_ififo_empty[thread_id][0].read() && wb_ififo_ren[thread_id][0].read()) {
        data_vector<tb_input_precision> temp_data = wb_ififo_rdata[thread_id][0].read();
        if (temp_data.size() != 0 && temp_data[3].range(0, 0) == 1) {
          if (current_tag[thread_id].read() == MAX_TAG - 1)
            sim_log.log(error, "Tag value is overflowing!", this->name());
          current_tag[thread_id].write(current_tag[thread_id].read() + 1);
          sim_log.log(sim_trace,
                      "Thread " + std::to_string(thread_id) + " tag is updated to " +
                          std::to_string(current_tag[thread_id].read() + 1),
                      this->name());
        }
      }
    }

    if (!sector_stall) {
    unsigned int thread_id = active_thread.read();
    bool ofifo_wen_val = false;
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
      ofifo_wen_val = ofifo_wen_val || ofifo_wen_signal[thread_id].read();
    }
    if (ofifo_wen_val && inst_pipeline[SECTOR_INST_PIPELINE - 2].read().last_uop &&
        (inst_fifo_empty[thread_id].read() || (current_tag[thread_id] < inst_fifo_rdata[thread_id].read().tag))) {
      thread_locked.write(false);
    }

    if (!thread_locked.read()) {
      for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
        unsigned int nxt_thread_id = thread_order.front();
        thread_order.push(nxt_thread_id);
        thread_order.pop();
        if (!inst_fifo_empty[nxt_thread_id].read() &&
            (current_tag[nxt_thread_id] >= inst_fifo_rdata[nxt_thread_id].read().tag)) {
          active_thread.write(nxt_thread_id);
          thread_locked.write(true);
          break;
        }
      }
    }
    }

    // Perform sequential logic of the tile module only if tile is not stalled for back-pressure
    unsigned int thread_id = active_thread.read();
    if (!sector_stall) {
      // Set the first stage of all pipelines
      for (unsigned int core_id = 0; core_id < CORES; core_id++) {
        for (unsigned int tile_id = 0; tile_id < TILES; tile_id++)
          vrf_rdata_pipeline[core_id][tile_id][0].write(vrf_rdata_signal[thread_id][core_id][tile_id].read());
      }
      mrf_raddr_pipeline[0].write(inst_fifo_rdata[thread_id].read().mrf_addr.to_uint());
      inst_pipeline[0].write(inst_fifo_rdata[thread_id].read());
      inst_valid_pipeline[0].write(inst_fifo_ren[thread_id].read());
      thread_id_pipeline[0].write(thread_id);
      for (unsigned int core_id = 0; core_id < CORES; core_id++) {
        data_vector<tb_output_precision> dpe_results_reshaped =
            data_vector<tb_output_precision>(TB_NUM_DOTS * DPES_PER_SECTOR);
        for (unsigned int tile_id = 0; tile_id < TILES; tile_id++) {
          for (unsigned int element_id = 0; element_id < TB_NUM_DOTS * DPES_PER_SECTOR; element_id++) {
            data_vector<tb_output_precision> dpe_results_temp =
                dpe_result_signal[core_id][element_id % DPES_PER_SECTOR][tile_id].read();
            if (dpe_results_temp.size() != 0) {
              dpe_results_reshaped[element_id] =
                  dpe_results_reshaped[element_id] + dpe_results_temp[element_id / DPES_PER_SECTOR];
            }
          }
          reduction_pipeline[0][core_id].write(data_vector<tb_output_precision>(dpe_results_reshaped));
        }
      }
      reduction_valid_pipeline[0].write(dpe_result_valid_signal[0][0][0]);

      // Advance all pipelines
      for (unsigned int core_id = 0; core_id < CORES; core_id++)
        for (unsigned int stage_id = 1; stage_id < SECTOR_VRF_DATA_PIPELINE; stage_id++)
          for (unsigned int tile_id = 0; tile_id < TILES; tile_id++)
            vrf_rdata_pipeline[core_id][tile_id][stage_id].write(
                vrf_rdata_pipeline[core_id][tile_id][stage_id - 1].read());
      for (unsigned int stage_id = 1; stage_id < SECTOR_MRF_ADDR_PIPELINE; stage_id++)
        mrf_raddr_pipeline[stage_id].write(mrf_raddr_pipeline[stage_id - 1]);
      for (unsigned int stage_id = 1; stage_id < SECTOR_INST_PIPELINE; stage_id++) {
        inst_pipeline[stage_id].write(inst_pipeline[stage_id - 1].read());
        thread_id_pipeline[stage_id].write(thread_id_pipeline[stage_id - 1].read());
      }
      for (unsigned int stage_id = 1; stage_id < SECTOR_INST_TO_DPES_PIPELINE; stage_id++)
        inst_valid_pipeline[stage_id].write(inst_valid_pipeline[stage_id - 1].read());
      for (unsigned int core_id = 0; core_id < CORES; core_id++) {
        for (unsigned int stage_id = 1; stage_id < SECTOR_REDUCTION_PIPELINE + SECTOR_REDUCTION_TO_ACCUM_PIPELINE;
             stage_id++) {
          reduction_pipeline[stage_id][core_id].write(reduction_pipeline[stage_id - 1][core_id].read());
          reduction_valid_pipeline[stage_id].write(reduction_valid_pipeline[stage_id - 1].read());
        }
        for (unsigned int stage_id = 1; stage_id < SECTOR_ACCUM_TO_OFIFO_PIPELINE; stage_id++) {
          accum_to_ofifo_pipeline[stage_id][core_id].write(accum_to_ofifo_pipeline[stage_id - 1][core_id].read());
          accum_to_ofifo_valid_pipeline[stage_id].write(accum_to_ofifo_valid_pipeline[stage_id - 1].read());
        }
      }
    }

    /*std::cout << this->name() << " ";
    for(unsigned int i = 0; i < THREADS; i++)
      std::cout << current_tag[i] << "(" << inst_fifo_empty[i] << ") - ";
    std::cout << sector_stall << " " << active_thread.read() << std::endl;*/
    wait();
  }
}

void mvu_sector::Assign() {
  if (rst.read()) {
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
      inst_fifo_rdy[thread_id].write(false);
      inst_fifo_ren[thread_id].write(false);
      for (unsigned int core_id = 0; core_id < CORES; core_id++) {
        sector_ofifo_rdy[thread_id][core_id].write(false);
        wb_fifo_ren[thread_id][core_id].write(false);
        wb_ififo_ren[thread_id][core_id].write(false);
        wb_ififo_wen[thread_id][core_id].write(false);
        for (unsigned int tile_id = 0; tile_id < TILES; tile_id++)
          vrf_wen_signal[thread_id][core_id][tile_id].write(false);
      }
    }
    sector_stall.write(false);
  } else {
    // Set MRF write address and write enable signals
    mrf_waddr_signal.write(mrf_waddr.read().to_uint());
    for (unsigned int dpe_id = 0; dpe_id < DPES_PER_SECTOR; dpe_id++)
      for (unsigned int tile_id = 0; tile_id < TILES; tile_id++)
        mrf_wen_signal[dpe_id][tile_id].write(
            mrf_wid.read() == ((tile_id * DPES_PER_SECTOR * SECTORS) + (sector_id * DPES_PER_SECTOR) + dpe_id));

    // Set instruction FIFO ready signal if FIFO is not almost full
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++)
      inst_fifo_rdy[thread_id].write(!inst_fifo_almost_full[thread_id].read());

    // Stall sector for back pressure if any of the output FIFOs is almost full
    bool all_ofifos_rdy = true;
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++)
      for (unsigned int core_id = 0; core_id < CORES; core_id++)
        all_ofifos_rdy &= !ofifo_almost_full[thread_id][core_id].read();
    sector_stall.write(!all_ofifos_rdy);

    // Issue an instruction if there is one ready, all output FIFOs are ready, and current tag value is greater than or
    // equal to the next instruction in queue
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) inst_fifo_ren[thread_id].write(false);
    unsigned int thread_id = active_thread.read();
    inst_fifo_ren[thread_id].write(!inst_fifo_empty[thread_id].read() && all_ofifos_rdy &&
                                   (current_tag[thread_id] >= inst_fifo_rdata[thread_id].read().tag));

    // Set VRF read address and ID based on instruction specifications
    vrf_raddr_signal.write(inst_fifo_rdata[thread_id].read().vrf_addr.to_uint());
    vrf_rd_id_signal.write(inst_fifo_rdata[thread_id].read().vrf_rd_id.to_uint());

    // Set DPE control signals
    dpe_valid_a_signal.write(inst_valid_pipeline[SECTOR_INST_TO_DPES_PIPELINE - 1]);
    dpe_valid_b_signal.write(inst_valid_pipeline[SECTOR_DISTRIBUTION_PIPELINE + SECTOR_MRF_DATA_PIPELINE - 1] &&
                             inst_pipeline[SECTOR_DISTRIBUTION_PIPELINE + SECTOR_MRF_DATA_PIPELINE - 1].read().vrf_en);
    dpe_reg_shift_sel_signal.write(
        inst_pipeline[SECTOR_DISTRIBUTION_PIPELINE + SECTOR_MRF_DATA_PIPELINE - 1].read().reg_sel);
    dpe_reg_use_sel_signal.write(inst_pipeline[SECTOR_INST_TO_DPES_PIPELINE - 1].read().reg_sel);

    accum_size_signal.write(inst_pipeline[SECTOR_INST_TO_ACCUM_PIPELINE - 1].read().accum_size);
    accum_op_signal.write(inst_pipeline[SECTOR_INST_TO_ACCUM_PIPELINE - 1].read().accum_op);

    // Push word to output FIFOs if a valid word reaches the end of the pipeline and all output FIFOs are ready
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) ofifo_wen_signal[thread_id].write(false);
    thread_id = thread_id_pipeline[SECTOR_INST_PIPELINE - 2].read();
    ofifo_wen_signal[thread_id].write(accum_to_ofifo_valid_pipeline[SECTOR_ACCUM_TO_OFIFO_PIPELINE - 1] &&
                                      all_ofifos_rdy);

    // Logic for the two write-back channels from the loader to this module
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
      for (unsigned int core_id = 0; core_id < CORES; core_id++) {
        wb_ififo_wen[thread_id][core_id].write(!wb_ififo_almost_full[thread_id][core_id].read() &&
                                               wb_fifo_rdy[thread_id][core_id].read());
        wb_fifo_ren[thread_id][core_id].write(!wb_ififo_almost_full[thread_id][core_id].read() &&
                                              wb_fifo_rdy[thread_id][core_id].read());

        sector_ofifo_rdy[thread_id][core_id].write(!ofifo_empty[thread_id][core_id].read());
        sector_chain_ofifo_rdy[thread_id][core_id].write(!sector_chain_ofifo_empty[thread_id][core_id].read());
        sector_chain_ofifo_wen[thread_id][core_id].write(wb_ififo_ren[thread_id][core_id] && (sector_id < SECTORS - 1));

        data_vector<tb_input_precision> wb_rdata = wb_ififo_rdata[thread_id][core_id].read();

        // Check if write-back FIFOs are ready and write incoming data to this tile's register files
        if ((wb_rdata.size() != 0) && !wb_ififo_empty[thread_id][core_id].read() &&
            !sector_chain_ofifo_almost_full[thread_id][core_id].read()) {
          tb_input_precision vrf_id = wb_rdata[0];
          wb_ififo_ren[thread_id][core_id].write(true);
          sc_uint<VRF_ADDRW> vrf_addr;
          vrf_addr.range(LOW_PRECISION - 1, 0) = wb_rdata[1];
          if (VRF_ADDRW > LOW_PRECISION)
            vrf_addr.range(VRF_ADDRW - 1, LOW_PRECISION) = wb_rdata[2].range(VRF_ADDRW - LOW_PRECISION - 1, 0);

          data_vector<tb_input_precision> vrf_wdata(LANES);
          for (unsigned int element_id = 0; element_id < LANES; element_id++) {
            vrf_wdata[element_id] = wb_rdata[4 + element_id];
          }
          vrf_wdata_signal[thread_id][core_id][vrf_id].write(vrf_wdata);

          vrf_waddr_signal[thread_id][core_id][vrf_id].write(vrf_addr);
          for (unsigned int tile_id = 0; tile_id < TILES; tile_id++)
            vrf_wen_signal[thread_id][core_id][tile_id].write(tile_id == vrf_id);
        } else {
          wb_ififo_ren[thread_id][core_id].write(false);
          for (unsigned int tile_id = 0; tile_id < TILES; tile_id++)
            vrf_wen_signal[thread_id][core_id][tile_id].write(false);
        }
      }
    }
  }
}