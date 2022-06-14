#include <evrf.hpp>

evrf::evrf(const sc_module_name& name, unsigned int id_sector, unsigned int id_thread)
    : sc_module(name),
      inst_fifo_full("inst_fifo_full"),
      inst_fifo_almost_full("inst_fifo_almost_full"),
      inst_fifo_empty("inst_fifo_empty"),
      inst_fifo_almost_empty("inst_fifo_almost_empty"),
      inst_fifo_ren("inst_fifo_ren"),
      inst_fifo_rdata("inst_fifo_rdata"),
      ififo_full("ififo_full", CORES),
      ififo_almost_full("ififo_almost_full", CORES),
      ififo_empty("ififo_empty", CORES),
      ififo_almost_empty("ififo_almost_empty", CORES),
      ififo_ren("ififo_ren", CORES),
      ififo_wen("ififo_wen", CORES),
      ififo_rdata("ififo_rdata", CORES),
      wb_ififo_full("wb_ififo_full", CORES),
      wb_ififo_almost_full("wb_ififo_almost_full", CORES),
      wb_ififo_empty("wb_ififo_empty", CORES),
      wb_ififo_almost_empty("wb_ififo_almost_empty", CORES),
      wb_ififo_ren("wb_ififo_ren", CORES),
      wb_ififo_wen("wb_ififo_wen", CORES),
      wb_ififo_rdata("wb_ififo_rdata", CORES),
      ofifo_full("ofifo_full", CORES),
      ofifo_almost_full("ofifo_almost_full", CORES),
      ofifo_empty("ofifo_empty", CORES),
      ofifo_almost_empty("ofifo_almost_empty", CORES),
      reset_uop("reset_uop"),
      vrf_wdata_signal("vrf_wdata_signal", CORES),
      vrf_rdata_signal("vrf_rdata_signal", CORES),
      vrf_raddr_signal("vrf_raddr_signal"),
      vrf_waddr_signal("vrf_waddr_signal"),
      vrf_wen_signal("vrf_wen_signal", CORES),
      vrf_clk_en("vrf_clk_en"),
      current_tag("current_tag"),
      inst_pipeline("inst_pipeline", EVRF_INST_PIPELINE),
      inst_valid_pipeline("inst_valid_pipeline", EVRF_INST_PIPELINE),
      input_data_pipeline("input_data_pipeline"),
      vrf_to_ofifo_data_pipeline("vrf_to_ofifo_data_pipeline"),
      vrf_to_ofifo_valid_pipeline("vrf_to_ofifo_valid_pipeline"),
      inst("inst"),
      inst_fifo_wen("inst_fifo_wen"),
      inst_fifo_rdy("inst_fifo_rdy"),
      wbfifo_rdy("wbfifo_rdy", CORES),
      wbfifo_ren("wbfifo_ren", CORES),
      wbfifo_rdata("wbfifo_rdata", CORES),
      mvu_ofifo_rdy("mvu_ofifo_rdy", CORES),
      mvu_ofifo_ren("mvu_ofifo_ren", CORES),
      mvu_ofifo_rdata("mvu_ofifo_rdata", CORES),
      evrf_ofifo_rdy("evrf_ofifo_rdy", CORES),
      evrf_ofifo_ren("evrf_ofifo_ren", CORES),
      evrf_result("evrf_result", CORES) {
        
  // Create SystemC variables with the required sizes
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(input_data_pipeline,
                                                                           EVRF_INST_TO_VRF + RF_RD_LATENCY, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(vrf_to_ofifo_data_pipeline,
                                                                           EVRF_RF_TO_OFIFO_PIPELINE, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(vrf_to_ofifo_valid_pipeline, EVRF_RF_TO_OFIFO_PIPELINE, CORES);

  sector_id = id_sector;
  thread_id = id_thread;

  // Create micro-op FIFO
  inst_fifo = new instruction_fifo<evrf_uop>("evrf_inst_fifo", UOP_FIFO_DEPTH, UOP_FIFO_ALMOST_FULL_DEPTH, 0);
  inst_fifo->clk(clk);
  inst_fifo->rst(rst);
  inst_fifo->wen(inst_fifo_wen);
  inst_fifo->ren(inst_fifo_ren);
  inst_fifo->wdata(inst);
  inst_fifo->full(inst_fifo_full);
  inst_fifo->almost_full(inst_fifo_almost_full);
  inst_fifo->empty(inst_fifo_empty);
  inst_fifo->almost_empty(inst_fifo_almost_empty);
  inst_fifo->rdata(inst_fifo_rdata);

  // Create register files and output FIFOs (one per NPU core)
  register_files.resize(CORES);
  ififos.resize(CORES);
  wb_ififos.resize(CORES);
  ofifos.resize(CORES);
  std::string rf_name_str, ofifo_name_str, ififo_name_str, wb_ififo_name_str, rf_init_filename;
  char rf_name[NAME_LENGTH], ififo_name[NAME_LENGTH], wb_ififo_name[NAME_LENGTH], ofifo_name[NAME_LENGTH];
  for (unsigned int core_id = 0; core_id < CORES; core_id++) {
    rf_name_str = "evrf_vrf_" + std::to_string(core_id);
    std::strcpy(rf_name, rf_name_str.c_str());
    register_files[core_id] =
        new register_file<tb_output_precision>(rf_name, SECTORS, VRF_DEPTH, DPES_PER_SECTOR, rf_init_filename);
    register_files[core_id]->clk(clk);
    register_files[core_id]->rst(rst);
    register_files[core_id]->raddr(vrf_raddr_signal);
    register_files[core_id]->wdata(vrf_wdata_signal[core_id]);
    register_files[core_id]->waddr(vrf_waddr_signal);
    register_files[core_id]->wen(vrf_wen_signal[core_id]);
    register_files[core_id]->rdata(vrf_rdata_signal[core_id]);
    register_files[core_id]->clk_en(vrf_clk_en);

    ififo_name_str = "evrf_ififo_" + std::to_string(core_id);
    std::strcpy(ififo_name, ififo_name_str.c_str());
    ififos[core_id] =
        new fifo<tb_output_precision>(ififo_name, DATA_FIFO_DEPTH, DPES_PER_SECTOR, DATA_FIFO_DEPTH - EVRF_PIPELINE, 0);
    ififos[core_id]->clk(clk);
    ififos[core_id]->rst(rst);
    ififos[core_id]->wen(ififo_wen[core_id]);
    ififos[core_id]->ren(ififo_ren[core_id]);
    ififos[core_id]->wdata(mvu_ofifo_rdata[core_id]);
    ififos[core_id]->full(ififo_full[core_id]);
    ififos[core_id]->almost_full(ififo_almost_full[core_id]);
    ififos[core_id]->empty(ififo_empty[core_id]);
    ififos[core_id]->almost_empty(ififo_almost_empty[core_id]);
    ififos[core_id]->rdata(ififo_rdata[core_id]);

    wb_ififo_name_str = "evrf_wb_ififo_" + std::to_string(core_id);
    std::strcpy(wb_ififo_name, wb_ififo_name_str.c_str());
    wb_ififos[core_id] = new fifo<tb_output_precision>(wb_ififo_name, DATA_FIFO_DEPTH, DPES_PER_SECTOR + 4,
                                                       DATA_FIFO_DEPTH - EVRF_PIPELINE, 0);
    wb_ififos[core_id]->clk(clk);
    wb_ififos[core_id]->rst(rst);
    wb_ififos[core_id]->wen(wb_ififo_wen[core_id]);
    wb_ififos[core_id]->ren(wb_ififo_ren[core_id]);
    wb_ififos[core_id]->wdata(wbfifo_rdata[core_id]);
    wb_ififos[core_id]->full(wb_ififo_full[core_id]);
    wb_ififos[core_id]->almost_full(wb_ififo_almost_full[core_id]);
    wb_ififos[core_id]->empty(wb_ififo_empty[core_id]);
    wb_ififos[core_id]->almost_empty(wb_ififo_almost_empty[core_id]);
    wb_ififos[core_id]->rdata(wb_ififo_rdata[core_id]);

    ofifo_name_str = "evrf_ofifo_" + std::to_string(core_id);
    std::strcpy(ofifo_name, ofifo_name_str.c_str());
    ofifos[core_id] =
        new fifo<tb_output_precision>(ofifo_name, DATA_FIFO_DEPTH, DPES_PER_SECTOR, DATA_FIFO_DEPTH - EVRF_PIPELINE, 0);
    ofifos[core_id]->clk(clk);
    ofifos[core_id]->rst(rst);
    ofifos[core_id]->wen(vrf_to_ofifo_valid_pipeline[EVRF_RF_TO_OFIFO_PIPELINE - 1][core_id]);
    ofifos[core_id]->ren(evrf_ofifo_ren[core_id]);
    ofifos[core_id]->wdata(vrf_to_ofifo_data_pipeline[EVRF_RF_TO_OFIFO_PIPELINE - 1][core_id]);
    ofifos[core_id]->full(ofifo_full[core_id]);
    ofifos[core_id]->almost_full(ofifo_almost_full[core_id]);
    ofifos[core_id]->empty(ofifo_empty[core_id]);
    ofifos[core_id]->almost_empty(ofifo_almost_empty[core_id]);
    ofifos[core_id]->rdata(evrf_result[core_id]);
  }

  // Initialize dummy micro-op for resetting the instruction pipeline to default (invalid) instruction
  reset_uop = sc_signal<evrf_uop>();

  // Setting sensitivity list for SC_METHOD & clock and reset signals for SC_CTHREAD
  SC_METHOD(Assign);
  sensitive << inst_fifo_full << inst_fifo_empty << current_tag << inst_fifo_rdata;
  for (unsigned int core_id = 0; core_id < CORES; core_id++) {
    sensitive << ofifo_empty[core_id] << mvu_ofifo_rdy[core_id] << ofifo_almost_full[core_id];
    sensitive << wbfifo_rdy[core_id] << wbfifo_rdata[core_id];
    sensitive << ififo_empty[core_id] << ififo_almost_full[core_id];
    sensitive << wb_ififo_empty[core_id] << wb_ififo_almost_full[core_id] << wb_ififo_rdata[core_id];
  }
  sensitive << rst << inst_pipeline[EVRF_INST_TO_VRF - 1] << inst_fifo_almost_full;
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

evrf::~evrf() {
  delete inst_fifo;
  for (unsigned int core_id = 0; core_id < CORES; core_id++) delete ofifos[core_id];
}

void evrf::Tick() {
  // Reset logic
  current_tag.write(0);
  for (unsigned int stage_id = 0; stage_id < EVRF_INST_PIPELINE; stage_id++) {
    inst_pipeline[stage_id].write(reset_uop);
    inst_valid_pipeline[stage_id].write(false);
  }
  vrf_clk_en.write(false);
  wait();

  // Sequential logic
  while (true) {
    // Update current tag value for hazard detection and resolution
    if (!wb_ififo_empty[0].read() && wb_ififo_ren[0].read()) {
      data_vector<tb_output_precision> temp_data = wb_ififo_rdata[0].read();
      if (temp_data.size() != 0 && temp_data[2].range(0, 0) == 1) {
        if (current_tag.read() == MAX_TAG - 1) sim_log.log(error, "Tag value is overflowing!", this->name());
        current_tag.write(current_tag.read() + 1);
        sim_log.log(sim_trace, "Tag is updated to " + std::to_string(current_tag.read() + 1), this->name());
        sim_trace_probe.record_event((sector_id * (NUM_PIPELINE_BLOCKS - 1)) + 1, TAG_UPDATE_TRACE);
      }
    }

    // Register first pipeline stages
    inst_pipeline[0].write(inst_fifo_rdata.read());
    inst_valid_pipeline[0].write(inst_fifo_ren);
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      input_data_pipeline[0][core_id].write(ififo_rdata[core_id].read());
      vrf_to_ofifo_data_pipeline[0][core_id].write(
          (inst_pipeline[EVRF_INST_TO_VRF + RF_RD_LATENCY - 1].read().src_sel == EVRF_FROM_VRF)
              ? vrf_rdata_signal[core_id].read()
              : input_data_pipeline[EVRF_INST_TO_VRF + RF_RD_LATENCY - 1][core_id].read());
      vrf_to_ofifo_valid_pipeline[0][core_id].write(
          inst_valid_pipeline[EVRF_INST_TO_VRF + RF_RD_LATENCY - 1].read() &&
          (inst_pipeline[EVRF_INST_TO_VRF + RF_RD_LATENCY - 1].read().src_sel != EVRF_FLUSH_MVU));
    }

    // Advance all pipelines
    for (unsigned int stage_id = 1; stage_id < EVRF_INST_PIPELINE; stage_id++) {
      inst_pipeline[stage_id].write(inst_pipeline[stage_id - 1].read());
      inst_valid_pipeline[stage_id].write(inst_valid_pipeline[stage_id - 1]);
    }
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      for (unsigned int stage_id = 1; stage_id < EVRF_INST_TO_VRF + RF_RD_LATENCY; stage_id++)
        input_data_pipeline[stage_id][core_id].write(input_data_pipeline[stage_id - 1][core_id].read());
      for (unsigned int stage_id = 1; stage_id < EVRF_RF_TO_OFIFO_PIPELINE; stage_id++) {
        vrf_to_ofifo_data_pipeline[stage_id][core_id].write(vrf_to_ofifo_data_pipeline[stage_id - 1][core_id].read());
        vrf_to_ofifo_valid_pipeline[stage_id][core_id].write(vrf_to_ofifo_valid_pipeline[stage_id - 1][core_id]);
      }
    }

    if (inst_fifo_ren.read()){
      sim_trace_probe.record_event((sector_id * (NUM_PIPELINE_BLOCKS - 1)) + 1, UOP_ISSUE_TRACE);
      //std::cout << this->name() << " read evrf inst" << std::endl;
    }
    if (vrf_to_ofifo_valid_pipeline[EVRF_RF_TO_OFIFO_PIPELINE - 1][0].read()){
      sim_trace_probe.record_event((sector_id * (NUM_PIPELINE_BLOCKS - 1)) + 1, UOP_RETIRE_TRACE);
      //std::cout << this->name() << " wrote to evrf ofifo" << std::endl;
    }
    if (inst_fifo_ren.read() && inst_fifo_rdata.read().first_uop)
      sim_trace_probe.record_event((sector_id * (NUM_PIPELINE_BLOCKS - 1)) + 1, FIRST_UOP_ISSUE_TRACE);
    if (vrf_to_ofifo_valid_pipeline[EVRF_RF_TO_OFIFO_PIPELINE - 1][0].read() &&
        inst_pipeline[EVRF_INST_PIPELINE - 1].read().last_uop)
      sim_trace_probe.record_event((sector_id * (NUM_PIPELINE_BLOCKS - 1)) + 1, LAST_UOP_RETIRE_TRACE);

    wait();
  }
}

void evrf::Assign() {
  // Reset logic
  if (rst.read()) {
    inst_fifo_rdy.write(false);
    inst_fifo_ren.write(false);
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      mvu_ofifo_ren[core_id].write(false);
      evrf_ofifo_rdy[core_id].write(false);
      ififo_wen[core_id].write(false);
      ififo_ren[core_id].write(false);
      wb_ififo_wen[core_id].write(false);
      wb_ififo_ren[core_id].write(false);
    }
    // Combinational logic
  } else {
    // Set the instruction FIFO ready signal if intruction FIFO is not almost full
    inst_fifo_rdy.write(!inst_fifo_almost_full.read());

    // Issue a micro-instruction if there is: (1) available instructions to execute, (2) available space in the
    // output FIFO, (3) hazard detection is not a problem, and (4) data source is MVU and there is available data
    // in MVU output FIFO or source is VRF
    inst_fifo_ren.write(
        !inst_fifo_empty.read() && !ofifo_almost_full[0].read() && (current_tag >= inst_fifo_rdata.read().tag) &&
        (((inst_fifo_rdata.read().src_sel == EVRF_FROM_MVU || inst_fifo_rdata.read().src_sel == EVRF_FLUSH_MVU) &&
          !ififo_empty[0].read()) ||
         (inst_fifo_rdata.read().src_sel == EVRF_FROM_VRF)));

    // Pop a data word from MVU output FIFO
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      mvu_ofifo_ren[core_id].write(!ififo_almost_full[core_id].read() && mvu_ofifo_rdy[core_id].read());
      ififo_wen[core_id].write(!ififo_almost_full[core_id].read() && mvu_ofifo_rdy[core_id].read());
      ififo_ren[core_id].write(
          !inst_fifo_empty.read() && !ofifo_almost_full[core_id].read() &&
          (current_tag >= inst_fifo_rdata.read().tag) &&
          ((inst_fifo_rdata.read().src_sel == EVRF_FROM_MVU || inst_fifo_rdata.read().src_sel == EVRF_FLUSH_MVU) &&
           !ififo_empty[core_id].read()));
    }

    // Set VRF read address signal depending on the intruction field in the right pipeline stage (when reading the
    // register file occurs)
    vrf_raddr_signal.write(inst_pipeline[EVRF_INST_TO_VRF - 1].read().vrf_addr.to_uint());

    // Logic for the two write-back channels from the loader to this module
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      data_vector<tb_output_precision> wb_rdata = wb_ififo_rdata[core_id].read();
      // Set all of the read/write enable to false initially, some of them might be set to true later
      wb_ififo_ren[core_id].write(false);
      vrf_wen_signal[core_id].write(false);

      data_vector<tb_output_precision> temp = wbfifo_rdata[core_id].read();
      if (temp.size() != 0) {
        wb_ififo_wen[core_id].write(wbfifo_rdy[core_id].read() && !wb_ififo_almost_full[core_id].read() &&
                                    ((temp[0] >> 2) == 1));
        wbfifo_ren[core_id].write(wbfifo_rdy[core_id].read() && !wb_ififo_almost_full[core_id].read() &&
                                  ((temp[0] >> 2) == 1));
      } else {
        wb_ififo_wen[core_id].write(false);
        wbfifo_ren[core_id].write(false);
      }

      // Check if write-back FIFOs are ready and write incoming data to the register files
      if (!wb_ififo_empty[core_id].read() && (wb_rdata.size() != 0)) {
        tb_output_precision vrf_id = wb_rdata[0];
        wb_ififo_ren[core_id].write(true);
        sc_uint<VRF_ADDRW> vrf_addr = wb_rdata[1].range(VRF_ADDRW - 1, 0).to_uint();
        data_vector<tb_output_precision> vrf_wdata(DPES_PER_SECTOR);
        for (unsigned int element_id = 0; element_id < DPES_PER_SECTOR; element_id++) {
          vrf_wdata[element_id] = wb_rdata[4 + element_id];
        }
        vrf_wdata_signal[core_id].write(vrf_wdata);
        vrf_waddr_signal.write(vrf_addr);
        vrf_wen_signal[core_id].write(true);
      }
    }

    // Set the external VRF ready signals if output FIFOs are not empty
    for (unsigned int core_id = 0; core_id < CORES; core_id++)
      evrf_ofifo_rdy[core_id].write(!ofifo_empty[core_id].read());
  }
}
