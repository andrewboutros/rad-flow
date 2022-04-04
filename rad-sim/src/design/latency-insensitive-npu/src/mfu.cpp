#include <mfu.hpp>

// Helper function for executing the compute operations of the MFU depending on the instruction specifications. The
// activation functions are implemented as identity (i.e., output is the same as input) since the NPU frontend does
// not model the hardware approximation of these non-linear functions. Once this limitation is addressed in the NPU
// frontend, the part marked with "TODO" below can be change to reflect the functionality of these operations.
void mfu_compute(sc_vector<sc_signal<data_vector<tb_output_precision>>> &input_data, sc_signal<mfu_uop> &inst,
                 sc_vector<sc_signal<data_vector<tb_output_precision>>> &add_vrf_data,
                 sc_vector<sc_signal<data_vector<tb_output_precision>>> &mult_vrf_data,
                 sc_vector<sc_signal<data_vector<tb_output_precision>>> &compute_result) {
  std::vector<data_vector<tb_output_precision>> res;
  res.resize(CORES);
  for (unsigned int core_id = 0; core_id < CORES; core_id++)
    res[core_id] = data_vector<tb_output_precision>(DPES_PER_SECTOR);

  switch (inst.read().activation_op) {  // TODO: activations all implemented "functionally" as identity
    case MFU_TANH_OP:
      for (unsigned int core_id = 0; core_id < CORES; core_id++) res[core_id] = input_data[core_id].read();
      break;
    case MFU_SIGMOID_OP:
      for (unsigned int core_id = 0; core_id < CORES; core_id++) res[core_id] = input_data[core_id].read();
      break;
    case MFU_RELU_OP:
      for (unsigned int core_id = 0; core_id < CORES; core_id++) res[core_id] = input_data[core_id].read();
      break;
    default:
      for (unsigned int core_id = 0; core_id < CORES; core_id++) res[core_id] = input_data[core_id].read();
      break;
  }

  switch (inst.read().add_op) {
    case MFU_ADD_OP:
      for (unsigned int core_id = 0; core_id < CORES; core_id++)
        res[core_id] = res[core_id] + add_vrf_data[core_id].read();
      break;
    case MFU_SUB_OP:
      for (unsigned int core_id = 0; core_id < CORES; core_id++)
        res[core_id] = res[core_id] - add_vrf_data[core_id].read();
      break;
    case MFU_RSUB_OP:
      for (unsigned int core_id = 0; core_id < CORES; core_id++)
        res[core_id] = add_vrf_data[core_id].read() - res[core_id];
      break;
    case MFU_MAX_OP:
      for (unsigned int core_id = 0; core_id < CORES; core_id++)
        res[core_id] = max(res[core_id], add_vrf_data[core_id].read());
      break;
    default:
      for (unsigned int core_id = 0; core_id < CORES; core_id++) res[core_id] = res[core_id];
      break;
  }

  switch (inst.read().mult_op) {
    case MFU_MUL_OP:
      for (unsigned int core_id = 0; core_id < CORES; core_id++)
        res[core_id] = res[core_id] * mult_vrf_data[core_id].read();
      break;
    default:
      for (unsigned int core_id = 0; core_id < CORES; core_id++) res[core_id] = res[core_id];
      break;
  }

  for (unsigned int core_id = 0; core_id < CORES; core_id++) compute_result[core_id].write(res[core_id]);
}

mfu::mfu(const sc_module_name &name, unsigned int id, unsigned int id_sector, unsigned int id_thread)
    : sc_module(name),
      inst_fifo_wen("inst_fifo_wen"),
      inst_fifo_rdy("inst_fifo_rdy"),
      wbfifo_rdy("wbfifo_rdy", CORES),
      wbfifo_ren("wbfifo_ren", CORES),
      wbfifo_rdata("wbfifo_rdata", CORES),
      input_ofifo_rdy("input_ofifo_rdy", CORES),
      input_ofifo_ren("input_ofifo_ren", CORES),
      input_ofifo_rdata("input_ofifo_rdata", CORES),
      mfu_ofifo_rdy("mfu_ofifo_rdy", CORES),
      mfu_ofifo_ren("mfu_ofifo_ren", CORES),
      mfu_result("mfu_result", CORES),
      inst_fifo_rdata("inst_fifo_rdata"),
      inst_pipeline("inst_pipeline", MFU_PIPELINE),
      inst_valid_pipeline("inst_valid_pipeline", MFU_PIPELINE),
      input_to_compute_data_pipeline("input_to_compute_data_pipeline"),
      add_vrf_to_compute_data_pipeline("add_vrf_to_compute_data_pipeline"),
      mult_vrf_to_compute_data_pipeline("mult_vrf_to_compute_data_pipeline"),
      compute_data_pipeline("compute_data_pipeline"),
      compute_to_ofifo_data_pipeline("compute_to_ofifo_data_pipeline"),
      add_vrf_wdata_signal("add_vrf_wdata_signal", CORES),
      add_vrf_rdata_signal("add_vrf_rdata_signal", CORES),
      add_vrf_waddr_signal("add_vrf_waddr_signal", CORES),
      add_vrf_wen_signal("add_vrf_wen_signal", CORES),
      mult_vrf_wdata_signal("mult_vrf_wdata_signal", CORES),
      mult_vrf_rdata_signal("mult_vrf_rdata_signal", CORES),
      mult_vrf_waddr_signal("mult_vrf_waddr_signal", CORES),
      mult_vrf_wen_signal("mult_vrf_wen_signal", CORES),
      ofifo_full("ofifo_full", CORES),
      ofifo_almost_full("ofifo_almost_full", CORES),
      ofifo_empty("ofifo_empty", CORES),
      ofifo_almost_empty("ofifo_almost_empty", CORES),
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
      wb_ififo_rdata("wb_ififo_rdata", CORES) {
  mfu_id = id;
  sector_id = id_sector;
  thread_id = id_thread;

  // Create SystemC 2D vectors with the required sizes
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(input_to_compute_data_pipeline,
                                                                           MFU_INPUT_TO_COMPUTE_PIPELINE, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(add_vrf_to_compute_data_pipeline,
                                                                           MFU_VRFS_TO_COMPUTE_PIPELINE, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(mult_vrf_to_compute_data_pipeline,
                                                                           MFU_VRFS_TO_COMPUTE_PIPELINE, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(compute_data_pipeline, MFU_COMPUTE_PIPELINE,
                                                                           CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(compute_to_ofifo_data_pipeline,
                                                                           MFU_COMPUTE_TO_OFIFO_PIPELINE, CORES);

  // Create MFU instruction FIFO
  inst_fifo = new instruction_fifo<mfu_uop>("mfu_inst_fifo", UOP_FIFO_DEPTH, UOP_FIFO_ALMOST_FULL_DEPTH, 0);
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

  // Create an addition register file, a multiplication register, and an output FIFO per NPU core
  add_register_files.resize(CORES);
  mult_register_files.resize(CORES);
  ififos.resize(CORES);
  wb_ififos.resize(CORES);
  ofifos.resize(CORES);
  std::string add_rf_name_str, mult_rf_name_str, ofifo_name_str, ififo_name_str, wb_ififo_name_str,
      add_rf_init_filename, mult_rf_init_filename;
  char add_rf_name[NAME_LENGTH], mult_rf_name[NAME_LENGTH], ififo_name[NAME_LENGTH], wb_ififo_name[NAME_LENGTH],
      ofifo_name[NAME_LENGTH];
  for (unsigned int core_id = 0; core_id < CORES; core_id++) {
    ififo_name_str = "mfu" + std::to_string(mfu_id) + "_ififo_" + std::to_string(core_id);
    std::strcpy(ififo_name, ififo_name_str.c_str());
    ififos[core_id] =
        new fifo<tb_output_precision>(ififo_name, DATA_FIFO_DEPTH, DPES_PER_SECTOR, DATA_FIFO_DEPTH - MFU_PIPELINE, 0);
    ififos[core_id]->clk(clk);
    ififos[core_id]->rst(rst);
    ififos[core_id]->wen(ififo_wen[core_id]);
    ififos[core_id]->ren(ififo_ren[core_id]);
    ififos[core_id]->wdata(input_ofifo_rdata[core_id]);
    ififos[core_id]->full(ififo_full[core_id]);
    ififos[core_id]->almost_full(ififo_almost_full[core_id]);
    ififos[core_id]->empty(ififo_empty[core_id]);
    ififos[core_id]->almost_empty(ififo_almost_empty[core_id]);
    ififos[core_id]->rdata(ififo_rdata[core_id]);

    wb_ififo_name_str = "mfu" + std::to_string(mfu_id) + "_wb_ififo_" + std::to_string(core_id);
    std::strcpy(wb_ififo_name, wb_ififo_name_str.c_str());
    wb_ififos[core_id] = new fifo<tb_output_precision>(wb_ififo_name, DATA_FIFO_DEPTH, DPES_PER_SECTOR + 4,
                                                       DATA_FIFO_DEPTH - MFU_PIPELINE, 0);
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

    add_rf_name_str = "mfu" + std::to_string(mfu_id) + "_add_vrf_" + std::to_string(core_id);
    std::strcpy(add_rf_name, add_rf_name_str.c_str());
    add_register_files[core_id] = new register_file<tb_output_precision>(
        add_rf_name, SECTORS + (mfu_id * 2) + 1, VRF_DEPTH, DPES_PER_SECTOR, add_rf_init_filename);
    add_register_files[core_id]->clk(clk);
    add_register_files[core_id]->rst(rst);
    add_register_files[core_id]->raddr(add_vrf_raddr_signal);
    add_register_files[core_id]->wdata(add_vrf_wdata_signal[core_id]);
    add_register_files[core_id]->waddr(add_vrf_waddr_signal[core_id]);
    add_register_files[core_id]->wen(add_vrf_wen_signal[core_id]);
    add_register_files[core_id]->rdata(add_vrf_rdata_signal[core_id]);
    add_register_files[core_id]->clk_en(vrf_clk_en);

    mult_rf_name_str = "mfu" + std::to_string(mfu_id) + "_mult_vrf_" + std::to_string(core_id);
    std::strcpy(mult_rf_name, mult_rf_name_str.c_str());
    mult_register_files[core_id] = new register_file<tb_output_precision>(
        mult_rf_name, SECTORS + (mfu_id * 2) + 2, VRF_DEPTH, DPES_PER_SECTOR, mult_rf_init_filename);
    mult_register_files[core_id]->clk(clk);
    mult_register_files[core_id]->rst(rst);
    mult_register_files[core_id]->raddr(mult_vrf_raddr_signal);
    mult_register_files[core_id]->wdata(mult_vrf_wdata_signal[core_id]);
    mult_register_files[core_id]->waddr(mult_vrf_waddr_signal[core_id]);
    mult_register_files[core_id]->wen(mult_vrf_wen_signal[core_id]);
    mult_register_files[core_id]->rdata(mult_vrf_rdata_signal[core_id]);
    mult_register_files[core_id]->clk_en(vrf_clk_en);

    ofifo_name_str = "mfu" + std::to_string(mfu_id) + "_ofifo_" + std::to_string(core_id);
    std::strcpy(ofifo_name, ofifo_name_str.c_str());
    ofifos[core_id] =
        new fifo<tb_output_precision>(ofifo_name, DATA_FIFO_DEPTH, DPES_PER_SECTOR, DATA_FIFO_DEPTH - MFU_PIPELINE, 0);
    ofifos[core_id]->clk(clk);
    ofifos[core_id]->rst(rst);
    ofifos[core_id]->wen(inst_valid_pipeline[MFU_PIPELINE - 1]);
    ofifos[core_id]->ren(mfu_ofifo_ren[core_id]);
    ofifos[core_id]->wdata(compute_to_ofifo_data_pipeline[MFU_COMPUTE_TO_OFIFO_PIPELINE - 1][core_id]);
    ofifos[core_id]->full(ofifo_full[core_id]);
    ofifos[core_id]->almost_full(ofifo_almost_full[core_id]);
    ofifos[core_id]->empty(ofifo_empty[core_id]);
    ofifos[core_id]->almost_empty(ofifo_almost_empty[core_id]);
    ofifos[core_id]->rdata(mfu_result[core_id]);
  }

  // Initialize dummy (default/invalid) instruction for resetting instruction pipeline
  reset_uop = sc_signal<mfu_uop>();

  // Set sensitivity list of SC_METHOD & clock and reset signal of SC_CTHREAD
  SC_METHOD(Assign);
  sensitive << inst_fifo_full << inst_fifo_empty << current_tag << inst_fifo_rdata
            << inst_pipeline[MFU_INST_TO_VRFS_PIPLINE - 1] << inst_fifo_almost_full << rst;
  for (unsigned int core_id = 0; core_id < CORES; core_id++) {
    sensitive << ofifo_empty[core_id] << input_ofifo_rdy[core_id] << ofifo_almost_full[core_id];
    sensitive << wbfifo_rdy[core_id] << wbfifo_rdata[core_id];
    sensitive << ififo_empty[core_id] << ififo_almost_full[core_id];
    sensitive << wb_ififo_empty[core_id] << wb_ififo_almost_full[core_id] << wb_ififo_rdata[core_id];
  }
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

mfu::~mfu() {
  delete inst_fifo;
  for (unsigned int core_id = 0; core_id < CORES; core_id++) {
    delete add_register_files[core_id];
    delete mult_register_files[core_id];
    delete ififos[core_id];
    delete wb_ififos[core_id];
    delete ofifos[core_id];
  }
}

void mfu::Tick() {
  // Reset logic
  current_tag.write(0);
  for (unsigned int stage_id = 0; stage_id < MFU_PIPELINE; stage_id++) {
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
        if (current_tag.read() == MAX_TAG - 1) npu_log.log(error, "Tag value is overflowing!", this->name());
        current_tag.write(current_tag.read() + 1);
        npu_log.log(sim_trace, "Tag is updated to " + std::to_string(current_tag.read() + 1), this->name());
#ifdef GEN_TRACES
        npu_trace_probe.record_event((sector_id * (NUM_PIPELINE_BLOCKS - 1)) + 2 + mfu_id, TAG_UPDATE_TRACE);
#endif
      }
    }

    // Register and advance instruction pipelines
    inst_pipeline[0].write(inst_fifo_rdata.read());
    inst_valid_pipeline[0].write(inst_fifo_ren);
    for (unsigned int stage_id = 1; stage_id < MFU_PIPELINE; stage_id++) {
      inst_pipeline[stage_id].write(inst_pipeline[stage_id - 1].read());
      inst_valid_pipeline[stage_id].write(inst_valid_pipeline[stage_id - 1]);
    }

    // Perform MFU computations
    mfu_compute(input_to_compute_data_pipeline[MFU_INPUT_TO_COMPUTE_PIPELINE - 1],
                inst_pipeline[MFU_INPUT_TO_COMPUTE_PIPELINE - 1],
                add_vrf_to_compute_data_pipeline[MFU_VRFS_TO_COMPUTE_PIPELINE - 1],
                mult_vrf_to_compute_data_pipeline[MFU_VRFS_TO_COMPUTE_PIPELINE - 1], compute_data_pipeline[0]);

    // Register and advance data pipelines
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      input_to_compute_data_pipeline[0][core_id].write(ififo_rdata[core_id].read());
      add_vrf_to_compute_data_pipeline[0][core_id].write(add_vrf_rdata_signal[core_id].read());
      mult_vrf_to_compute_data_pipeline[0][core_id].write(mult_vrf_rdata_signal[core_id].read());
      compute_to_ofifo_data_pipeline[0][core_id].write(compute_data_pipeline[MFU_COMPUTE_PIPELINE - 1][core_id].read());

      for (unsigned int stage_id = 1; stage_id < MFU_INPUT_TO_COMPUTE_PIPELINE; stage_id++)
        input_to_compute_data_pipeline[stage_id][core_id].write(
            input_to_compute_data_pipeline[stage_id - 1][core_id].read());
      for (unsigned int stage_id = 1; stage_id < MFU_VRFS_TO_COMPUTE_PIPELINE; stage_id++) {
        add_vrf_to_compute_data_pipeline[stage_id][core_id].write(
            add_vrf_to_compute_data_pipeline[stage_id - 1][core_id].read());
        mult_vrf_to_compute_data_pipeline[stage_id][core_id].write(
            mult_vrf_to_compute_data_pipeline[stage_id - 1][core_id].read());
      }
      for (unsigned int stage_id = 1; stage_id < MFU_COMPUTE_PIPELINE; stage_id++)
        compute_data_pipeline[stage_id][core_id].write(compute_data_pipeline[stage_id - 1][core_id].read());
      for (unsigned int stage_id = 1; stage_id < MFU_COMPUTE_TO_OFIFO_PIPELINE; stage_id++)
        compute_to_ofifo_data_pipeline[stage_id][core_id].write(
            compute_to_ofifo_data_pipeline[stage_id - 1][core_id].read());
    }

#ifdef GEN_TRACES
    if (inst_fifo_ren.read())
      npu_trace_probe.record_event((sector_id * (NUM_PIPELINE_BLOCKS - 1)) + 2 + mfu_id, UOP_ISSUE_TRACE);
    if (inst_valid_pipeline[MFU_PIPELINE - 1].read())
      npu_trace_probe.record_event((sector_id * (NUM_PIPELINE_BLOCKS - 1)) + 2 + mfu_id, UOP_RETIRE_TRACE);
    if (inst_fifo_ren.read() && inst_fifo_rdata.read().first_uop)
      npu_trace_probe.record_event((sector_id * (NUM_PIPELINE_BLOCKS - 1)) + 2 + mfu_id, FIRST_UOP_ISSUE_TRACE);
    if (inst_valid_pipeline[MFU_PIPELINE - 1].read() && inst_pipeline[MFU_PIPELINE - 1].read().last_uop)
      npu_trace_probe.record_event((sector_id * (NUM_PIPELINE_BLOCKS - 1)) + 2 + mfu_id, LAST_UOP_RETIRE_TRACE);
#endif

    /*if(ififo_wen[0] && mfu_id == 1 && sector_id == 4) {
      count1++;
      cout << this->name() << " wrote to IFIFO -- " << count1 << endl;
    }
    if(mfu_ofifo_ren[0].read() && mfu_id == 0 && sector_id == 4) {
      count++;
      cout << this->name() << " read from OFIFO -- " << count << endl;
    }*/
    /*if(inst_fifo_wen && mfu_id == 1 && sector_id == 3) {
      count++;
      cout << this->name() << " Got uOP -- " << count << endl;
    }*/
    // if (mfu_id == 1 && sector_id == 2)
    //  cout << !ififo_empty[0].read() << " " << !inst_fifo_empty.read() << " " << endl;
    wait();
  }
}

void mfu::Assign() {
  // Reset logic
  if (rst.read()) {
    inst_fifo_rdy.write(false);
    inst_fifo_ren.write(false);
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      input_ofifo_ren[core_id].write(false);
      mfu_ofifo_rdy[core_id].write(false);
      ififo_wen[core_id].write(false);
      ififo_ren[core_id].write(false);
      wb_ififo_wen[core_id].write(false);
      wb_ififo_ren[core_id].write(false);
    }
    // Combinational logic
  } else {
    // Set the instruction FIFO ready signal if FIFO is not almost full
    inst_fifo_rdy.write(!inst_fifo_almost_full.read());
    // Issue a micro-instruction if there is: (1) available instructions to execute, (2) available space in the
    // output FIFO, (3) hazard detection is not a problem, and (4) available data in previous module output FIFO
    inst_fifo_ren.write(!inst_fifo_empty.read() && !ofifo_almost_full[0].read() &&
                        (current_tag >= inst_fifo_rdata.read().tag) && !ififo_empty[0].read());
    // Pop a data word from MVU output FIFO
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      input_ofifo_ren[core_id].write(input_ofifo_rdy[core_id].read() && !ififo_almost_full[core_id].read());
      ififo_wen[core_id].write(input_ofifo_rdy[core_id].read() && !ififo_almost_full[core_id].read());
      ififo_ren[core_id].write(!inst_fifo_empty.read() && !ofifo_almost_full[0].read() &&
                               (current_tag >= inst_fifo_rdata.read().tag) && !ififo_empty[core_id].read());
    }

    // Set VRF read addresses depending on the values specified by the MFU instructions
    add_vrf_raddr_signal.write(inst_pipeline[MFU_INST_TO_VRFS_PIPLINE - 1].read().vrf0_addr.to_uint());
    mult_vrf_raddr_signal.write(inst_pipeline[MFU_INST_TO_VRFS_PIPLINE - 1].read().vrf1_addr.to_uint());

    // Logic for the two write-back channels from the loader to this module
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      data_vector<tb_output_precision> wb_rdata = wb_ififo_rdata[core_id].read();
      // Set all of the read/write enable to false initially, some of them might be set to true later
      add_vrf_wen_signal[core_id].write(false);
      mult_vrf_wen_signal[core_id].write(false);
      wb_ififo_ren[core_id].write(false);

      data_vector<tb_output_precision> temp = wbfifo_rdata[core_id].read();
      if (temp.size() != 0) {
        unsigned int dest_node;
#ifdef RADSIM
        std::string dest_name =
            "axis_mfu" + std::to_string(mfu_id) + "_" + std::to_string(thread_id) + "_" + std::to_string(sector_id);
        dest_node = module_assignment[dest_name];
#else
        dest_node = (sector_id * 6) + MFU0_DEST_ID + mfu_id;
#endif
        wb_ififo_wen[core_id].write(!wb_ififo_almost_full[core_id].read() && wbfifo_rdy[core_id].read() &&
                                    ((temp[0] >> 2) == (2 + mfu_id)));
        wbfifo_ren[core_id].write(!wb_ififo_almost_full[core_id].read() && wbfifo_rdy[core_id].read() &&
                                  ((temp[0] >> 2) == (2 + mfu_id)));
      } else {
        wb_ififo_wen[core_id].write(false);
        wbfifo_ren[core_id].write(false);
      }

      // Check if write-back FIFOs are ready and write incoming data to the addition register files
      if (!wb_ififo_empty[core_id].read() && (wb_rdata.size() != 0) && (wb_rdata[0].range(1,0) == 1 || wb_rdata[0].range(1,0) == 3)) {
        wb_ififo_ren[core_id].write(true);
        sc_uint<VRF_ADDRW> vrf_addr = wb_rdata[1].range(VRF_ADDRW - 1, 0).to_uint();
        data_vector<tb_output_precision> vrf_wdata(DPES_PER_SECTOR);
        for (unsigned int element_id = 0; element_id < DPES_PER_SECTOR; element_id++) {
          vrf_wdata[element_id] = wb_rdata[4 + element_id];
        }
        add_vrf_wdata_signal[core_id].write(vrf_wdata);
        add_vrf_waddr_signal[core_id].write(vrf_addr);
        add_vrf_wen_signal[core_id].write(true);
        // if(vrf_addr < 511)
        //  cout << "Addition VRF waddr = " << vrf_addr << endl;
      }

      // Check if write-back FIFOs are ready and write incoming data to the multiplication register files
      if (!wb_ififo_empty[core_id].read() && (wb_rdata.size() != 0) && (wb_rdata[0].range(1,0) > 1)) {
        wb_ififo_ren[core_id].write(true);
        sc_uint<VRF_ADDRW> vrf_addr = wb_rdata[1].range(VRF_ADDRW - 1, 0).to_uint();
        data_vector<tb_output_precision> vrf_wdata(DPES_PER_SECTOR);
        for (unsigned int element_id = 0; element_id < DPES_PER_SECTOR; element_id++) {
          vrf_wdata[element_id] = wb_rdata[4 + element_id];
        }
        mult_vrf_wdata_signal[core_id].write(vrf_wdata);
        mult_vrf_waddr_signal[core_id].write(vrf_addr);
        mult_vrf_wen_signal[core_id].write(true);
      }
    }

    // Set the output FIFO ready signals if the FIFOs are not empty
    for (unsigned int core_id = 0; core_id < CORES; core_id++)
      mfu_ofifo_rdy[core_id].write(!ofifo_empty[core_id].read());
  }
}