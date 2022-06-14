#include <loader.hpp>

loader::loader(const sc_module_name& name)
    : sc_module(name),
      inst_fifo_full("inst_fifo_full"),
      inst_fifo_almost_full("inst_fifo_almost_full"),
      inst_fifo_empty("inst_fifo_empty"),
      inst_fifo_almost_empty("inst_fifo_almost_empty"),
      inst_fifo_ren("inst_fifo_ren"),
      inst_fifo_rdata("inst_fifo_rdata"),
      ext_input_fifo_full("ext_input_fifo_full", CORES),
      ext_input_fifo_almost_full("ext_input_fifo_almost_full", CORES),
      ext_input_fifo_empty("ext_input_fifo_empty", CORES),
      ext_input_fifo_almost_empty("ext_input_fifo_almost_empty", CORES),
      ext_input_fifo_ren("ext_input_fifo_ren", CORES),
      ext_input_fifo_rdata("ext_input_fifo_rdata", CORES),
      ext_output_fifo_full("ext_output_fifo_full", CORES),
      ext_output_fifo_almost_full("ext_output_fifo_almost_full", CORES),
      ext_output_fifo_empty("ext_output_fifo_empty", CORES),
      ext_output_fifo_almost_empty("ext_output_fifo_almost_empty", CORES),
      wb0_fifo_full("wb0_fifo_full", CORES),
      wb0_fifo_almost_full("wb0_fifo_almost_full", CORES),
      wb0_fifo_empty("wb0_fifo_empty", CORES),
      wb0_fifo_almost_empty("wb0_fifo_almost_empty", CORES),
      wb1_fifo_full("wb1_fifo_full"),
      wb1_fifo_almost_full("wb1_fifo_almost_full"),
      wb1_fifo_empty("wb1_fifo_empty"),
      wb1_fifo_almost_empty("wb1_fifo_almost_empty"),
      wb0_fifo_wdata("wb0_fifo_wdata", CORES),
      wb1_fifo_wdata("wb1_fifo_wdata"),
      ififo_full("ififo_full"),
      ififo_almost_full("ififo_almost_full"),
      ififo_empty("ififo_empty"),
      ififo_almost_empty("ififo_almost_empty"),
      ififo_ren("ififo_ren"),
      ififo_rdata("ififo_rdata"),
      reset_uop("reset_uop"),
      inst_pipeline("inst_pipeline", LD_PIPELINE),
      inst_valid_pipeline("inst_valid_pipeline", LD_PIPELINE),
      ext_output_fifo_wen_pipeline("ext_output_fifo_wen_pipeline", LD_PIPELINE),
      wb0_fifo_wen_pipeline("wb0_fifo_wen_pipeline", LD_PIPELINE),
      wb1_fifo_wen_pipeline("wb1_fifo_wen_pipeline", LD_PIPELINE),
      data_pipeline("data_pipeline"),
      inst("inst"),
      inst_fifo_wen("inst_fifo_wen"),
      inst_fifo_rdy("inst_fifo_rdy"),
      wb0_fifo_rdy("wb0_fifo_rdy", CORES),
      wb0_fifo_ren("wb0_fifo_ren", CORES),
      wb0_fifo_rdata("wb0_fifo_rdata", CORES),
      wb1_fifo_rdy("wb1_fifo_rdy"),
      wb1_fifo_ren("wb1_fifo_ren"),
      wb1_fifo_rdata("wb1_fifo_rdata"),
      input_fifo_rdy("input_fifo_rdy"),
      input_fifo_ren("input_fifo_ren"),
      input_fifo_rdata("input_fifo_rdata"),
      ext_input_fifo_rdy("ext_input_fifo_rdy", CORES),
      ext_input_fifo_wen("ext_input_fifo_wen", CORES),
      ext_input_fifo_wdata("ext_input_fifo_wdata", CORES),
      ext_output_fifo_rdy("ext_output_fifo_rdy", CORES),
      ext_output_fifo_ren("ext_output_fifo_ren", CORES),
      ext_output_fifo_rdata("ext_output_fifo_rdata", CORES) {

  // Create SystemC 2D vectors with the required sizes
  init_vector<sc_in<bool>>::init_sc_vector(input_fifo_rdy, SECTORS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(input_fifo_ren, SECTORS, CORES);
  init_vector<sc_in<data_vector<tb_output_precision>>>::init_sc_vector(input_fifo_rdata, SECTORS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(wb1_fifo_rdy, SECTORS, CORES);
  init_vector<sc_in<bool>>::init_sc_vector(wb1_fifo_ren, SECTORS, CORES);
  init_vector<sc_out<data_vector<tb_output_precision>>>::init_sc_vector(wb1_fifo_rdata, SECTORS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(wb1_fifo_wdata, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(wb1_fifo_full, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(wb1_fifo_almost_full, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(wb1_fifo_empty, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(wb1_fifo_almost_empty, SECTORS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(data_pipeline, LD_PIPELINE, CORES);

  init_vector<sc_signal<bool>>::init_sc_vector(ififo_full, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ififo_almost_full, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ififo_empty, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ififo_almost_empty, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ififo_ren, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ififo_wen, SECTORS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(ififo_rdata, SECTORS, CORES);

  // Create loader instruction FIFO
  inst_fifo = new instruction_fifo<ld_uop>("ld_inst_fifo", UOP_FIFO_DEPTH, UOP_FIFO_ALMOST_FULL_DEPTH, 0);
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

  // Create input, output and two channels of write-back FIFOs (one per NPU core)
  ext_input_fifos.resize(CORES);
  ext_output_fifos.resize(CORES);
  wb0_fifos.resize(CORES);
  wb1_fifos.resize(SECTORS);
  ififos.resize(SECTORS);
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    wb1_fifos[sector_id].resize(CORES);
    ififos[sector_id].resize(CORES);
  }

  std::string ext_input_fifo_name_str, ext_output_fifo_name_str, wb0_fifo_name_str, wb1_fifo_name_str, ififo_name_str;
  char ext_input_fifo_name[NAME_LENGTH], ext_output_fifo_name[NAME_LENGTH], wb0_fifo_name[NAME_LENGTH],
      wb1_fifo_name[NAME_LENGTH], ififo_name[NAME_LENGTH];
  for (unsigned int core_id = 0; core_id < CORES; core_id++) {
    ext_input_fifo_name_str = "ld_ext_input_fifo_" + std::to_string(core_id);
    std::strcpy(ext_input_fifo_name, ext_input_fifo_name_str.c_str());
    ext_input_fifos[core_id] =
        new fifo<tb_input_precision>(ext_input_fifo_name, DATA_FIFO_DEPTH, LANES, DATA_FIFO_DEPTH - LD_PIPELINE, 0);
    ext_input_fifos[core_id]->clk(clk);
    ext_input_fifos[core_id]->rst(rst);
    ext_input_fifos[core_id]->wen(ext_input_fifo_wen[core_id]);
    ext_input_fifos[core_id]->ren(ext_input_fifo_ren[core_id]);
    ext_input_fifos[core_id]->wdata(ext_input_fifo_wdata[core_id]);
    ext_input_fifos[core_id]->full(ext_input_fifo_full[core_id]);
    ext_input_fifos[core_id]->almost_full(ext_input_fifo_almost_full[core_id]);
    ext_input_fifos[core_id]->empty(ext_input_fifo_empty[core_id]);
    ext_input_fifos[core_id]->almost_empty(ext_input_fifo_almost_empty[core_id]);
    ext_input_fifos[core_id]->rdata(ext_input_fifo_rdata[core_id]);

    ext_output_fifo_name_str = "ld_ext_output_fifo_" + std::to_string(core_id);
    std::strcpy(ext_output_fifo_name, ext_output_fifo_name_str.c_str());
    ext_output_fifos[core_id] =
        new fifo<tb_output_precision>(ext_output_fifo_name, DATA_FIFO_DEPTH, LANES, DATA_FIFO_DEPTH - LD_PIPELINE, 0);
    ext_output_fifos[core_id]->clk(clk);
    ext_output_fifos[core_id]->rst(rst);
    ext_output_fifos[core_id]->wen(ext_output_fifo_wen_pipeline[LD_PIPELINE - 1]);
    ext_output_fifos[core_id]->ren(ext_output_fifo_ren[core_id]);
    ext_output_fifos[core_id]->wdata(data_pipeline[LD_PIPELINE - 1][core_id]);
    ext_output_fifos[core_id]->full(ext_output_fifo_full[core_id]);
    ext_output_fifos[core_id]->almost_full(ext_output_fifo_almost_full[core_id]);
    ext_output_fifos[core_id]->empty(ext_output_fifo_empty[core_id]);
    ext_output_fifos[core_id]->almost_empty(ext_output_fifo_almost_empty[core_id]);
    ext_output_fifos[core_id]->rdata(ext_output_fifo_rdata[core_id]);

    wb0_fifo_name_str = "ld_wb0_fifo_" + std::to_string(core_id);
    std::strcpy(wb0_fifo_name, wb0_fifo_name_str.c_str());
    wb0_fifos[core_id] =
        new fifo<tb_input_precision>(wb0_fifo_name, DATA_FIFO_DEPTH, LANES + 4, DATA_FIFO_DEPTH - LD_PIPELINE, 0);
    wb0_fifos[core_id]->clk(clk);
    wb0_fifos[core_id]->rst(rst);
    wb0_fifos[core_id]->wen(wb0_fifo_wen_pipeline[LD_PIPELINE - 1]);
    wb0_fifos[core_id]->ren(wb0_fifo_ren[core_id]);
    wb0_fifos[core_id]->wdata(wb0_fifo_wdata[core_id]);
    wb0_fifos[core_id]->full(wb0_fifo_full[core_id]);
    wb0_fifos[core_id]->almost_full(wb0_fifo_almost_full[core_id]);
    wb0_fifos[core_id]->empty(wb0_fifo_empty[core_id]);
    wb0_fifos[core_id]->almost_empty(wb0_fifo_almost_empty[core_id]);
    wb0_fifos[core_id]->rdata(wb0_fifo_rdata[core_id]);

    for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
      ififo_name_str = "ld_ififo_" + std::to_string(sector_id) + "_" + std::to_string(core_id);
      std::strcpy(ififo_name, ififo_name_str.c_str());
      ififos[sector_id][core_id] = new fifo<tb_output_precision>(ififo_name, DATA_FIFO_DEPTH, DPES_PER_SECTOR + 5,
                                                                 DATA_FIFO_DEPTH - LD_PIPELINE, 0);
      ififos[sector_id][core_id]->clk(clk);
      ififos[sector_id][core_id]->rst(rst);
      ififos[sector_id][core_id]->wen(ififo_wen[sector_id][core_id]);
      ififos[sector_id][core_id]->ren(ififo_ren[sector_id][core_id]);
      ififos[sector_id][core_id]->wdata(input_fifo_rdata[sector_id][core_id]);
      ififos[sector_id][core_id]->full(ififo_full[sector_id][core_id]);
      ififos[sector_id][core_id]->almost_full(ififo_almost_full[sector_id][core_id]);
      ififos[sector_id][core_id]->empty(ififo_empty[sector_id][core_id]);
      ififos[sector_id][core_id]->almost_empty(ififo_almost_empty[sector_id][core_id]);
      ififos[sector_id][core_id]->rdata(ififo_rdata[sector_id][core_id]);

      wb1_fifo_name_str = "ld_wb1_fifo_" + std::to_string(sector_id) + "_" + std::to_string(core_id);
      std::strcpy(wb1_fifo_name, wb1_fifo_name_str.c_str());
      wb1_fifos[sector_id][core_id] = new fifo<tb_output_precision>(wb1_fifo_name, DATA_FIFO_DEPTH, DPES_PER_SECTOR + 4,
                                                                    DATA_FIFO_DEPTH - LD_PIPELINE, 0);
      wb1_fifos[sector_id][core_id]->clk(clk);
      wb1_fifos[sector_id][core_id]->rst(rst);
      wb1_fifos[sector_id][core_id]->wen(wb1_fifo_wen_pipeline[LD_PIPELINE - 1]);
      wb1_fifos[sector_id][core_id]->ren(wb1_fifo_ren[sector_id][core_id]);
      wb1_fifos[sector_id][core_id]->wdata(wb1_fifo_wdata[sector_id][core_id]);
      wb1_fifos[sector_id][core_id]->full(wb1_fifo_full[sector_id][core_id]);
      wb1_fifos[sector_id][core_id]->almost_full(wb1_fifo_almost_full[sector_id][core_id]);
      wb1_fifos[sector_id][core_id]->empty(wb1_fifo_empty[sector_id][core_id]);
      wb1_fifos[sector_id][core_id]->almost_empty(wb1_fifo_almost_empty[sector_id][core_id]);
      wb1_fifos[sector_id][core_id]->rdata(wb1_fifo_rdata[sector_id][core_id]);
    }
  }

  // Initialize dummy (default/invalid) instruction for resetting instruction pipeline
  reset_uop = sc_signal<ld_uop>();

  // Set sensitivity list of SC_METHOD & clock and reset signals of SC_CTHREAD
  SC_METHOD(Assign);
  sensitive << inst_fifo_full << inst_fifo_empty << inst_fifo_rdata << inst_fifo_almost_full;
  for (unsigned int core_id = 0; core_id < CORES; core_id++) {
    sensitive << ext_output_fifo_empty[core_id] << ext_output_fifo_almost_full[core_id] << ext_input_fifo_full[core_id]
              << ext_input_fifo_rdy[core_id] << ext_input_fifo_almost_full[core_id] << ext_input_fifo_empty[core_id]
              << wb0_fifo_empty[core_id] << wb0_fifo_almost_full[core_id];
    for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
      sensitive << input_fifo_rdy[sector_id][core_id] << wb1_fifo_empty[sector_id][core_id]
                << wb1_fifo_almost_full[sector_id][core_id];
      sensitive << ififo_empty[sector_id][core_id] << ififo_almost_full[sector_id][core_id];
    }
  }
  sensitive << rst;
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

loader::~loader() {
  delete inst_fifo;
  for (unsigned int core_id = 0; core_id < CORES; core_id++) {
    delete ext_input_fifos[core_id];
    delete ext_output_fifos[core_id];
    delete wb0_fifos[core_id];
    for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
      delete wb1_fifos[sector_id][core_id];
      delete ififos[sector_id][core_id];
    }
  }
}

void pack_data_and_dest_info_ch0(sc_signal<ld_uop>& ld_inst, data_vector<tb_output_precision>& write_back_data_vectors,
                                 data_vector<tb_input_precision>& write_back_vector) {
  // Pack the destination info in the first three entries of the vector: destination VRF ID, write address, and last
  // flag (indicating whether it is a tag update message or not)
  write_back_vector[0] = ld_inst.read().vrf0_id - 1;
  write_back_vector[1] = ld_inst.read().vrf0_addr.range(LOW_PRECISION - 1, 0);
  if (VRF_ADDRW > LOW_PRECISION) write_back_vector[2] = ld_inst.read().vrf0_addr.range(VRF_ADDRW - 1, LOW_PRECISION);
  write_back_vector[3] = (ld_inst.read().last) ? 1 : 0;

  // Set the remaining entries of the vector with corresponding data entries for the write back vector
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    for (unsigned int element_id = 0; element_id < DPES_PER_SECTOR; element_id++) {
      write_back_vector[4 + (sector_id * DPES_PER_SECTOR) + element_id] =
          write_back_data_vectors[(sector_id * DPES_PER_SECTOR) + element_id];
    }
  }
}

void pack_data_and_dest_info_ch1(sc_signal<ld_uop>& ld_inst, data_vector<tb_output_precision>& write_back_data_vectors,
                                 std::vector<data_vector<tb_output_precision>>& write_back_vectors) {
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    // Pack the destination info in the first three entries of the vector: destination VRF ID, write address, and last
    // flag (indicating whether it is a tag update message or not)
    data_vector<tb_output_precision> write_back_temp(DPES_PER_SECTOR + 4);
    write_back_temp[0] = ld_inst.read().vrf1_id;
    write_back_temp[1] = ld_inst.read().vrf1_addr;
    write_back_temp[2] = (ld_inst.read().last) ? 1 : 0;

    std::string dest_name;
    if (ld_inst.read().block1_id == 1) {
      write_back_temp[0] += (1 << 2);
    } else if (ld_inst.read().block1_id == 2) {
      write_back_temp[0] += (2 << 2);
    } else {
      write_back_temp[0] += (3 << 2);
    }

    // Set the remaining entries of the vector with corresponding data entries for the write back vector
    for (unsigned int element_id = 0; element_id < DPES_PER_SECTOR; element_id++) {
      write_back_temp[4 + element_id] = write_back_data_vectors[(sector_id * DPES_PER_SECTOR) + element_id];
    }
    write_back_vectors[sector_id] = write_back_temp;
  }
}

void loader::Tick() {
  // Reset logic
  for (unsigned int stage_id = 0; stage_id < LD_PIPELINE; stage_id++) {
    inst_pipeline[stage_id].write(reset_uop);
    inst_valid_pipeline[stage_id].write(false);
    ext_output_fifo_wen_pipeline[stage_id].write(false);
    wb0_fifo_wen_pipeline[stage_id].write(false);
    wb1_fifo_wen_pipeline[stage_id].write(false);
  }
  wait();

  // Sequential logic
  while (true) {
    // Prepare the write back vector one cycle before the end of the loader pipeline to be presented to the write back
    // FIFOs the next cycle
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      data_vector<tb_output_precision> data_from_pipeline = data_pipeline[LD_PIPELINE - 2][core_id].read();

      // Pack data and destination info for each write back channel if there is a valid load instruction and the VRF
      // ID of this instruction is not zero (i.e., writing back to one of the VRFs)
      if (inst_valid_pipeline[LD_PIPELINE - 2].read() && inst_pipeline[LD_PIPELINE - 2].read().vrf0_id != 0 &&
          data_from_pipeline.size() != 0) {
        data_vector<tb_input_precision> write_back_vector(LANES + 4);
        pack_data_and_dest_info_ch0(inst_pipeline[LD_PIPELINE - 2], data_from_pipeline, write_back_vector);
        wb0_fifo_wdata[core_id].write(write_back_vector);
      }

      if (inst_valid_pipeline[LD_PIPELINE - 2].read() && inst_pipeline[LD_PIPELINE - 2].read().vrf1_id != 0 &&
          data_from_pipeline.size() != 0) {
        std::vector<data_vector<tb_output_precision>> write_back_vectors(SECTORS);
        for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
          data_vector<tb_output_precision> write_back_vector(DPES_PER_SECTOR + 4);
          write_back_vectors[sector_id] = write_back_vector;
        }
        pack_data_and_dest_info_ch1(inst_pipeline[LD_PIPELINE - 2], data_from_pipeline, write_back_vectors);
        for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++)
          wb1_fifo_wdata[sector_id][core_id].write(write_back_vectors[sector_id]);
      }
    }

    // Register and advance instruction (control) pipelines
    inst_pipeline[0].write(inst_fifo_rdata.read());
    inst_valid_pipeline[0].write(inst_fifo_ren.read());
    ext_output_fifo_wen_pipeline[0].write(inst_fifo_ren.read() && inst_fifo_rdata.read().output_result);
    wb0_fifo_wen_pipeline[0].write(inst_fifo_ren.read() && (inst_fifo_rdata.read().vrf0_id != 0));
    wb1_fifo_wen_pipeline[0].write(inst_fifo_ren.read() && (inst_fifo_rdata.read().vrf1_id != 0));
    for (unsigned int stage_id = 1; stage_id < LD_PIPELINE; stage_id++) {
      inst_pipeline[stage_id].write(inst_pipeline[stage_id - 1].read());
      inst_valid_pipeline[stage_id].write(inst_valid_pipeline[stage_id - 1]);
      ext_output_fifo_wen_pipeline[stage_id].write(ext_output_fifo_wen_pipeline[stage_id - 1]);
      wb0_fifo_wen_pipeline[stage_id].write(wb0_fifo_wen_pipeline[stage_id - 1]);
      wb1_fifo_wen_pipeline[stage_id].write(wb1_fifo_wen_pipeline[stage_id - 1]);
    }

    // Register data pipeline depending on the source specified by the instruction and advance data pipelines
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      if (inst_fifo_rdata.read().src_sel == LD_FROM_IN && ext_input_fifo_ren[core_id].read()) {
        data_vector<tb_output_precision> temp(LANES);
        data_vector<tb_input_precision> ext_input = ext_input_fifo_rdata[core_id].read();
        for (unsigned int element_id = 0; element_id < LANES; element_id++)
          temp[element_id] = ext_input[element_id].to_int();
        data_pipeline[0][core_id].write(temp);
      } else if (inst_fifo_rdata.read().src_sel == LD_FROM_WB && ififo_ren[0][core_id].read()) {
        data_vector<tb_output_precision> temp(LANES);
        for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
          data_vector<tb_output_precision> sector_input = ififo_rdata[sector_id][core_id].read();
          for (unsigned int element_id = 0; element_id < DPES_PER_SECTOR; element_id++)
            temp[(sector_id * DPES_PER_SECTOR) + element_id] = sector_input[element_id];
        }
        data_pipeline[0][core_id].write(temp);
      }
      for (unsigned int stage_id = 1; stage_id < LD_PIPELINE; stage_id++)
        data_pipeline[stage_id][core_id].write(data_pipeline[stage_id - 1][core_id].read());
    }

    if (inst_fifo_ren.read()) sim_trace_probe.record_event(SECTORS * (NUM_PIPELINE_BLOCKS - 1), UOP_ISSUE_TRACE);
    if (ext_output_fifo_wen_pipeline[LD_PIPELINE - 1].read() || wb0_fifo_wen_pipeline[LD_PIPELINE - 1].read() ||
        wb1_fifo_wen_pipeline[LD_PIPELINE - 1].read())
      sim_trace_probe.record_event(SECTORS * (NUM_PIPELINE_BLOCKS - 1), UOP_RETIRE_TRACE);
    if (inst_fifo_ren.read() && inst_fifo_rdata.read().first_uop)
      sim_trace_probe.record_event(SECTORS * (NUM_PIPELINE_BLOCKS - 1), FIRST_UOP_ISSUE_TRACE);
    if ((ext_output_fifo_wen_pipeline[LD_PIPELINE - 1].read() || wb0_fifo_wen_pipeline[LD_PIPELINE - 1].read() ||
         wb1_fifo_wen_pipeline[LD_PIPELINE - 1].read()) &&
        inst_pipeline[LD_PIPELINE - 1].read().last_uop)
      sim_trace_probe.record_event(SECTORS * (NUM_PIPELINE_BLOCKS - 1), LAST_UOP_RETIRE_TRACE);

    wait();
  }
}

void loader::Assign() {
  if (rst.read()) {
    inst_fifo_rdy.write(false);
    inst_fifo_ren.write(false);
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
        input_fifo_ren[sector_id][core_id].write(false);
        ififo_ren[sector_id][core_id].write(false);
        ififo_wen[sector_id][core_id].write(false);
        wb1_fifo_rdy[sector_id][core_id].write(false);
      }
      ext_input_fifo_ren[core_id].write(false);
      ext_output_fifo_rdy[core_id].write(false);
      wb0_fifo_rdy[core_id].write(false);
    }
  } else {
    // Set instruction FIFO ready signal if instruction FIFO is not almost full
    inst_fifo_rdy.write(!inst_fifo_almost_full.read());

    for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
      for (unsigned int core_id = 0; core_id < CORES; core_id++) {
        ififo_wen[sector_id][core_id].write(input_fifo_rdy[sector_id][core_id].read() &&
                                            !ififo_almost_full[sector_id][core_id].read());
        input_fifo_ren[sector_id][core_id].write(input_fifo_rdy[sector_id][core_id].read() &&
                                                 !ififo_almost_full[sector_id][core_id].read());
      }
    }

    // Get status of all FIFOs
    bool all_ext_input_fifos_rdy = true, all_input_fifos_rdy = true, all_ext_output_fifos_rdy = true,
         all_wb_ext_output_fifos_rdy = true;
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      all_ext_input_fifos_rdy &= !ext_input_fifo_empty[core_id].read();
      all_wb_ext_output_fifos_rdy &= !wb0_fifo_almost_full[core_id].read();
      all_ext_output_fifos_rdy &= !ext_output_fifo_almost_full[core_id].read();
      for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
        all_input_fifos_rdy &= !ififo_empty[sector_id][core_id].read();
        all_wb_ext_output_fifos_rdy &= !wb1_fifo_almost_full[sector_id][core_id].read();
      }
    }

    // Issume an instruction if there is one ready, all output FIFOs have space to accept data words, and one of the
    // following is true: (1) external inputs are ready in case of load instruction, or (2) write-back inputs are
    // ready in case of a write-back instruction, or (3) it is a tag update instruction
    inst_fifo_ren.write(!inst_fifo_empty.read() && all_ext_output_fifos_rdy && all_wb_ext_output_fifos_rdy &&
                        ((all_ext_input_fifos_rdy && inst_fifo_rdata.read().src_sel == LD_FROM_IN) ||
                         (all_input_fifos_rdy && inst_fifo_rdata.read().src_sel == LD_FROM_WB) ||
                         (inst_fifo_rdata.read().src_sel == LD_TAG_UPDATE)));

    // Set different read enable and ready signals appropriately for different FIFOs
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      ext_input_fifo_ren[core_id].write(!inst_fifo_empty.read() && all_ext_output_fifos_rdy &&
                                        all_wb_ext_output_fifos_rdy && all_ext_input_fifos_rdy &&
                                        (inst_fifo_rdata.read().src_sel == LD_FROM_IN));
      ext_output_fifo_rdy[core_id].write(!ext_output_fifo_empty[core_id].read());
      ext_input_fifo_rdy[core_id].write(!ext_input_fifo_almost_full[core_id].read());
      wb0_fifo_rdy[core_id].write(!wb0_fifo_empty[core_id].read());
      for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
        ififo_ren[sector_id][core_id].write(!inst_fifo_empty.read() && all_ext_output_fifos_rdy &&
                                            all_wb_ext_output_fifos_rdy && all_input_fifos_rdy &&
                                            (inst_fifo_rdata.read().src_sel == LD_FROM_WB));
        wb1_fifo_rdy[sector_id][core_id].write(!wb1_fifo_empty[sector_id][core_id].read());
      }
    }
  }
}