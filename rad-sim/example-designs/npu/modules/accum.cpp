#include "accum.hpp"

accum::accum(const sc_module_name& name)
    : sc_module(name),
      accum_in_pipeline("accum_in_pipeline"),
      accum_raddr_pipeline("accum_raddr_pipeline", ACCUM_PIPELINE),
      valid_in_pipeline("valid_in_pipeline", ACCUM_PIPELINE),
      accum_op_pipeline("accum_op_pipeline", ACCUM_PIPELINE),
      accum_wdata("accum_wdata", CORES),
      accum_rdata("accum_rdata", CORES),
      accum_in("accum_in", CORES),
      accum_out("accum_out", CORES),
      clk_en("clk_en") {
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(accum_in_pipeline, CORES, ACCUM_PIPELINE);

  accum_scratchpad.resize(CORES);
  for (unsigned int core_id = 0; core_id < CORES; core_id++) {
    accum_scratchpad[core_id].resize(2 * NUM_ACCUM);
    for (unsigned int word_id = 0; word_id < 2 * NUM_ACCUM; word_id++)
      accum_scratchpad[core_id][word_id].resize(TB_NUM_DOTS * DPES_PER_SECTOR);
  }

  SC_METHOD(Assign);
  sensitive << accum_op_pipeline[ACCUM_PIPELINE - 1] << valid_in_pipeline[ACCUM_PIPELINE - 1] << clk_en << rst;
  for (unsigned int core_id = 0; core_id < CORES; core_id++) sensitive << accum_wdata[core_id];

  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

accum::~accum() {}

void accum::Tick() {
  accum_raddr.write(0);
  for (unsigned int stage_id = 0; stage_id < ACCUM_PIPELINE; stage_id++) {
    accum_raddr_pipeline[stage_id].write(0);
    valid_in_pipeline[stage_id].write(false);
    accum_op_pipeline[stage_id].write(0);
  }
  wait();

  while (true) {
    if (!clk_en.read()) {
      if (accum_size.read() > (2 * NUM_ACCUM)) {
        std::cerr << "Input size exceeds accumulator memory size!" << std::endl;
        exit(1);
      }

      if (valid_in.read()) {
        if (accum_raddr.read() == accum_size.read() - 1)
          accum_raddr.write(0);
        else
          accum_raddr.write(accum_raddr.read() + 1);
      }

      for (unsigned int core_id = 0; core_id < CORES; core_id++) {
        accum_rdata[core_id].write(
            data_vector<tb_output_precision>(accum_scratchpad[core_id][accum_raddr_pipeline[ACCUM_PIPELINE - 3].read()]));
      }

      if ((accum_op_pipeline[ACCUM_PIPELINE - 2].read() == ACCUM_OP_SET) ||
          (accum_op_pipeline[ACCUM_PIPELINE - 2].read() == ACCUM_OP_SWB)) {
        for (unsigned int core_id = 0; core_id < CORES; core_id++) {
          accum_wdata[core_id].write(accum_in_pipeline[core_id][ACCUM_PIPELINE - 2].read());
        }
      } else {
        for (unsigned int core_id = 0; core_id < CORES; core_id++) {
          accum_wdata[core_id].write(accum_in_pipeline[core_id][ACCUM_PIPELINE - 2].read() + accum_rdata[core_id].read());
        }
      }

      if (valid_in_pipeline[ACCUM_PIPELINE - 1].read()) {
        for (unsigned int core_id = 0; core_id < CORES; core_id++) {
          data_vector<tb_output_precision> temp = accum_wdata[core_id].read();
          for (unsigned int element_id = 0; element_id < TB_NUM_DOTS * DPES_PER_SECTOR; element_id++)
            accum_scratchpad[core_id][accum_raddr_pipeline[ACCUM_PIPELINE - 1].read()][element_id] = temp[element_id];
        }
      }

      // Advance accumulator pipelines
      accum_raddr_pipeline[0].write(accum_raddr.read());
      valid_in_pipeline[0].write(valid_in.read());
      accum_op_pipeline[0].write(accum_op.read());
      for (unsigned int core_id = 0; core_id < CORES; core_id++) {
        accum_in_pipeline[core_id][0].write(accum_in[core_id].read());
      }
      for (unsigned int stage_id = 1; stage_id < ACCUM_PIPELINE; stage_id++) {
        accum_raddr_pipeline[stage_id].write(accum_raddr_pipeline[stage_id - 1].read());
        valid_in_pipeline[stage_id].write(valid_in_pipeline[stage_id - 1].read());
        accum_op_pipeline[stage_id].write(accum_op_pipeline[stage_id - 1].read());
        for (unsigned int core_id = 0; core_id < CORES; core_id++) {
          accum_in_pipeline[core_id][stage_id].write(accum_in_pipeline[core_id][stage_id - 1]);
        }
      }
    }
    wait();
  }
}

void accum::Assign() {
  if (rst.read()) {
    valid_out.write(false);
  } else {
    valid_out.write(!clk_en.read() && valid_in_pipeline[ACCUM_PIPELINE - 1].read() &&
                    ((accum_op_pipeline[ACCUM_PIPELINE - 1].read() == ACCUM_OP_WRB) ||
                    (accum_op_pipeline[ACCUM_PIPELINE - 1].read() == ACCUM_OP_SWB)));

    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      data_vector<tb_output_precision> accum_wdata_temp = accum_wdata[core_id].read();
      if (accum_wdata_temp.size() != 0) {
        accum_out[core_id].write(accum_wdata[core_id].read());
      }
    }
  }
}
