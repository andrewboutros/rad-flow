#include <decoders.hpp>

vliw_inst_dispatch::vliw_inst_dispatch(const sc_module_name& name)
    : sc_module(name),
      sector_pc("sector_pc"),
      evrf_pc("evrf_pc"),
      mfu0_pc("mfu0_pc"),
      mfu1_pc("mfu1_pc"),
      ld_pc("ld_pc"),
      sector_dispatch_flag("sector_dispatch_flag"),
      evrf_dispatch_flag("evrf_dispatch_flag"),
      mfu0_dispatch_flag("mfu0_dispatch_flag"),
      mfu1_dispatch_flag("mfu1_dispatch_flag"),
      ld_dispatch_flag("ld_dispatch_flag"),
      sector_mop_fifo_wen("sector_mop_fifo_wen"),
      sector_mop_fifo_full("sector_mop_fifo_full"),
      sector_mop_fifo_almost_full("sector_mop_fifo_almost_full"),
      sector_mop_fifo_empty("sector_mop_fifo_empty"),
      sector_mop_fifo_almost_empty("sector_mop_fifo_almost_empty"),
      sector_mop_fifo_wdata("sector_mop_fifo_wdata"),
      evrf_mop_fifo_wen("evrf_mop_fifo_wen"),
      evrf_mop_fifo_full("evrf_mop_fifo_full"),
      evrf_mop_fifo_almost_full("evrf_mop_fifo_almost_full"),
      evrf_mop_fifo_empty("evrf_mop_fifo_empty"),
      evrf_mop_fifo_almost_empty("evrf_mop_fifo_almost_empty"),
      evrf_mop_fifo_wdata("evrf_mop_fifo_wdata"),
      mfu0_mop_fifo_wen("mfu0_mop_fifo_wen"),
      mfu0_mop_fifo_full("mfu0_mop_fifo_full"),
      mfu0_mop_fifo_almost_full("mfu0_mop_fifo_almost_full"),
      mfu0_mop_fifo_empty("mfu0_mop_fifo_empty"),
      mfu0_mop_fifo_almost_empty("mfu0_mop_fifo_almost_empty"),
      mfu0_mop_fifo_wdata("mfu0_mop_fifo_wdata"),
      mfu1_mop_fifo_wen("mfu1_mop_fifo_wen"),
      mfu1_mop_fifo_full("mfu1_mop_fifo_full"),
      mfu1_mop_fifo_almost_full("mfu1_mop_fifo_almost_full"),
      mfu1_mop_fifo_empty("mfu1_mop_fifo_empty"),
      mfu1_mop_fifo_almost_empty("mfu1_mop_fifo_almost_empty"),
      mfu1_mop_fifo_wdata("mfu1_mop_fifo_wdata"),
      ld_mop_fifo_wen("ld_mop_fifo_wen"),
      ld_mop_fifo_full("ld_mop_fifo_full"),
      ld_mop_fifo_almost_full("ld_mop_fifo_almost_full"),
      ld_mop_fifo_empty("ld_mop_fifo_empty"),
      ld_mop_fifo_almost_empty("ld_mop_fifo_almost_empty"),
      ld_mop_fifo_wdata("ld_mop_fifo_wdata") {
  // Size the dispatcher instruction memory and other SystemC vectors needed. Macro-op related signals/ports are
  // defined as vectors of size 1 to connect to AXI-streaming adapters and just match the template used for data FIFOs
  // that connect to multiple NPU cores working in lockstep.
  inst_memory.resize(INST_MEMORY_DEPTH);

  init_vector<sc_out<bool>>::init_sc_vector(sector_mop_rdy, SECTORS, 1);
  init_vector<sc_in<bool>>::init_sc_vector(sector_mop_ren, SECTORS, 1);
  init_vector<sc_out<mvu_mop>>::init_sc_vector(sector_mop_rdata, SECTORS, 1);
  init_vector<sc_out<bool>>::init_sc_vector(evrf_mop_rdy, SECTORS, 1);
  init_vector<sc_in<bool>>::init_sc_vector(evrf_mop_ren, SECTORS, 1);
  init_vector<sc_out<evrf_mop>>::init_sc_vector(evrf_mop_rdata, SECTORS, 1);
  init_vector<sc_out<bool>>::init_sc_vector(mfu0_mop_rdy, SECTORS, 1);
  init_vector<sc_in<bool>>::init_sc_vector(mfu0_mop_ren, SECTORS, 1);
  init_vector<sc_out<mfu_mop>>::init_sc_vector(mfu0_mop_rdata, SECTORS, 1);
  init_vector<sc_out<bool>>::init_sc_vector(mfu1_mop_rdy, SECTORS, 1);
  init_vector<sc_in<bool>>::init_sc_vector(mfu1_mop_ren, SECTORS, 1);
  init_vector<sc_out<mfu_mop>>::init_sc_vector(mfu1_mop_rdata, SECTORS, 1);
  init_vector<sc_out<bool>>::init_sc_vector(ld_mop_rdy, 1);
  init_vector<sc_in<bool>>::init_sc_vector(ld_mop_ren, 1);
  init_vector<sc_out<ld_mop>>::init_sc_vector(ld_mop_rdata, 1);

  init_vector<sc_signal<unsigned int>>::init_sc_vector(sector_pc, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_dispatch_flag, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_mop_fifo_wen, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_mop_fifo_full, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_mop_fifo_almost_full, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_mop_fifo_empty, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_mop_fifo_almost_empty, SECTORS);
  init_vector<sc_signal<mvu_mop>>::init_sc_vector(sector_mop_fifo_wdata, SECTORS);

  init_vector<sc_signal<unsigned int>>::init_sc_vector(evrf_pc, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_dispatch_flag, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_mop_fifo_wen, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_mop_fifo_full, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_mop_fifo_almost_full, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_mop_fifo_empty, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_mop_fifo_almost_empty, SECTORS);
  init_vector<sc_signal<evrf_mop>>::init_sc_vector(evrf_mop_fifo_wdata, SECTORS);

  init_vector<sc_signal<unsigned int>>::init_sc_vector(mfu0_pc, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_dispatch_flag, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_mop_fifo_wen, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_mop_fifo_full, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_mop_fifo_almost_full, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_mop_fifo_empty, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_mop_fifo_almost_empty, SECTORS);
  init_vector<sc_signal<mfu_mop>>::init_sc_vector(mfu0_mop_fifo_wdata, SECTORS);

  init_vector<sc_signal<unsigned int>>::init_sc_vector(mfu1_pc, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_dispatch_flag, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_mop_fifo_wen, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_mop_fifo_full, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_mop_fifo_almost_full, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_mop_fifo_empty, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_mop_fifo_almost_empty, SECTORS);
  init_vector<sc_signal<mfu_mop>>::init_sc_vector(mfu1_mop_fifo_wdata, SECTORS);

  // Create macro-op FIFOs for different NPU blocks
  std::string module_name_str;
  char module_name[NAME_LENGTH];

  sector_mop_fifo.resize(SECTORS);
  evrf_mop_fifo.resize(SECTORS);
  mfu0_mop_fifo.resize(SECTORS);
  mfu1_mop_fifo.resize(SECTORS);

  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    module_name_str = "sector_" + std::to_string(sector_id) + "_mop_fifo";
    std::strcpy(module_name, module_name_str.c_str());
    sector_mop_fifo[sector_id] =
        new instruction_fifo<mvu_mop>(module_name, MOP_FIFO_DEPTH, MOP_FIFO_ALMOST_FULL_DEPTH, 0);
    sector_mop_fifo[sector_id]->clk(clk);
    sector_mop_fifo[sector_id]->rst(rst);
    sector_mop_fifo[sector_id]->wen(sector_mop_fifo_wen[sector_id]);
    sector_mop_fifo[sector_id]->ren(sector_mop_ren[sector_id][0]);
    sector_mop_fifo[sector_id]->wdata(sector_mop_fifo_wdata[sector_id]);
    sector_mop_fifo[sector_id]->full(sector_mop_fifo_full[sector_id]);
    sector_mop_fifo[sector_id]->almost_full(sector_mop_fifo_almost_full[sector_id]);
    sector_mop_fifo[sector_id]->empty(sector_mop_fifo_empty[sector_id]);
    sector_mop_fifo[sector_id]->almost_empty(sector_mop_fifo_almost_empty[sector_id]);
    sector_mop_fifo[sector_id]->rdata(sector_mop_rdata[sector_id][0]);

    module_name_str = "evrf_" + std::to_string(sector_id) + "_mop_fifo";
    std::strcpy(module_name, module_name_str.c_str());
    evrf_mop_fifo[sector_id] =
        new instruction_fifo<evrf_mop>(module_name, MOP_FIFO_DEPTH, MOP_FIFO_ALMOST_FULL_DEPTH, 0);
    evrf_mop_fifo[sector_id]->clk(clk);
    evrf_mop_fifo[sector_id]->rst(rst);
    evrf_mop_fifo[sector_id]->wen(evrf_mop_fifo_wen[sector_id]);
    evrf_mop_fifo[sector_id]->ren(evrf_mop_ren[sector_id][0]);
    evrf_mop_fifo[sector_id]->wdata(evrf_mop_fifo_wdata[sector_id]);
    evrf_mop_fifo[sector_id]->full(evrf_mop_fifo_full[sector_id]);
    evrf_mop_fifo[sector_id]->almost_full(evrf_mop_fifo_almost_full[sector_id]);
    evrf_mop_fifo[sector_id]->empty(evrf_mop_fifo_empty[sector_id]);
    evrf_mop_fifo[sector_id]->almost_empty(evrf_mop_fifo_almost_empty[sector_id]);
    evrf_mop_fifo[sector_id]->rdata(evrf_mop_rdata[sector_id][0]);

    module_name_str = "mfu0_" + std::to_string(sector_id) + "_mop_fifo";
    std::strcpy(module_name, module_name_str.c_str());
    mfu0_mop_fifo[sector_id] =
        new instruction_fifo<mfu_mop>(module_name, MOP_FIFO_DEPTH, MOP_FIFO_ALMOST_FULL_DEPTH, 0);
    mfu0_mop_fifo[sector_id]->clk(clk);
    mfu0_mop_fifo[sector_id]->rst(rst);
    mfu0_mop_fifo[sector_id]->wen(mfu0_mop_fifo_wen[sector_id]);
    mfu0_mop_fifo[sector_id]->ren(mfu0_mop_ren[sector_id][0]);
    mfu0_mop_fifo[sector_id]->wdata(mfu0_mop_fifo_wdata[sector_id]);
    mfu0_mop_fifo[sector_id]->full(mfu0_mop_fifo_full[sector_id]);
    mfu0_mop_fifo[sector_id]->almost_full(mfu0_mop_fifo_almost_full[sector_id]);
    mfu0_mop_fifo[sector_id]->empty(mfu0_mop_fifo_empty[sector_id]);
    mfu0_mop_fifo[sector_id]->almost_empty(mfu0_mop_fifo_almost_empty[sector_id]);
    mfu0_mop_fifo[sector_id]->rdata(mfu0_mop_rdata[sector_id][0]);

    module_name_str = "mfu1_" + std::to_string(sector_id) + "_mop_fifo";
    std::strcpy(module_name, module_name_str.c_str());
    mfu1_mop_fifo[sector_id] =
        new instruction_fifo<mfu_mop>(module_name, MOP_FIFO_DEPTH, MOP_FIFO_ALMOST_FULL_DEPTH, 0);
    mfu1_mop_fifo[sector_id]->clk(clk);
    mfu1_mop_fifo[sector_id]->rst(rst);
    mfu1_mop_fifo[sector_id]->wen(mfu1_mop_fifo_wen[sector_id]);
    mfu1_mop_fifo[sector_id]->ren(mfu1_mop_ren[sector_id][0]);
    mfu1_mop_fifo[sector_id]->wdata(mfu1_mop_fifo_wdata[sector_id]);
    mfu1_mop_fifo[sector_id]->full(mfu1_mop_fifo_full[sector_id]);
    mfu1_mop_fifo[sector_id]->almost_full(mfu1_mop_fifo_almost_full[sector_id]);
    mfu1_mop_fifo[sector_id]->empty(mfu1_mop_fifo_empty[sector_id]);
    mfu1_mop_fifo[sector_id]->almost_empty(mfu1_mop_fifo_almost_empty[sector_id]);
    mfu1_mop_fifo[sector_id]->rdata(mfu1_mop_rdata[sector_id][0]);
  }

  module_name_str = "ld_mop_fifo";
  std::strcpy(module_name, module_name_str.c_str());
  ld_mop_fifo = new instruction_fifo<ld_mop>(module_name, MOP_FIFO_DEPTH, MOP_FIFO_ALMOST_FULL_DEPTH, 0);
  ld_mop_fifo->clk(clk);
  ld_mop_fifo->rst(rst);
  ld_mop_fifo->wen(ld_mop_fifo_wen);
  ld_mop_fifo->ren(ld_mop_ren[0]);
  ld_mop_fifo->wdata(ld_mop_fifo_wdata);
  ld_mop_fifo->full(ld_mop_fifo_full);
  ld_mop_fifo->almost_full(ld_mop_fifo_almost_full);
  ld_mop_fifo->empty(ld_mop_fifo_empty);
  ld_mop_fifo->almost_empty(ld_mop_fifo_almost_empty);
  ld_mop_fifo->rdata(ld_mop_rdata[0]);

  // Set sensitivity list of SC_METHOD & clock and reset of SC_CTHREAD
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
  SC_METHOD(Assign);
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++)
    sensitive << sector_mop_fifo_empty[sector_id] << evrf_mop_fifo_empty[sector_id] << mfu0_mop_fifo_empty[sector_id]
              << mfu1_mop_fifo_empty[sector_id];
  sensitive << ld_mop_fifo_empty;
}

vliw_inst_dispatch::~vliw_inst_dispatch() {}

void vliw_inst_dispatch::dispatch_logic(sc_signal<bool>& dispatch_flag, sc_signal<unsigned int>& pc,
                                        sc_signal<bool>& mop_fifo_almost_full, sc_signal<bool>& mop_fifo_wen, bool op) {
  // If start signal was previously triggered, consider dispatching a macro-op
  if (dispatch_flag.read()) {
    // Dispatch only if receiving FIFO is ready & update address/flag, otherwise do nothing
    if (!mop_fifo_almost_full.read()) {
      mop_fifo_wen.write(op);
      // If program counter did not reach end, increment address otherwise stop dispatching
      if (pc.read() < end_pc.read()) {
        pc.write(pc.read() + 1);
      } else {
        pc.write(start_pc.read());
        dispatch_flag.write(false);
      }
    } else {
      mop_fifo_wen.write(false);
    }
    // If start signal is triggered, set the dispatch flag to true
  } else if (start.read()) {
    dispatch_flag.write(true);
    pc.write(start_pc.read());
    mop_fifo_wen.write(false);
  } else {
    mop_fifo_wen.write(false);
  }
}

void vliw_inst_dispatch::Tick() {
  // Reset logic
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    sector_pc[sector_id].write(0);
    sector_dispatch_flag[sector_id].write(false);
    evrf_pc[sector_id].write(0);
    evrf_dispatch_flag[sector_id].write(false);
    mfu0_pc[sector_id].write(0);
    mfu0_dispatch_flag[sector_id].write(false);
    mfu1_pc[sector_id].write(0);
    mfu1_dispatch_flag[sector_id].write(false);
  }
  ld_pc.write(0);
  ld_dispatch_flag.write(false);
  wait();

  // Sequential logic
  while (true) {
    // Write VLIW instructions to the instruction memory
    if (inst_wen.read()) inst_memory[inst_waddr.read()] = inst_wdata.read();

    for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
      // Dispatch sector macro-ops
      dispatch_logic(sector_dispatch_flag[sector_id], sector_pc[sector_id], sector_mop_fifo_almost_full[sector_id],
                     sector_mop_fifo_wen[sector_id], inst_memory[sector_pc[sector_id].read()].mvu_inst.op);
      sector_mop_fifo_wdata[sector_id].write(inst_memory[sector_pc[sector_id].read()].mvu_inst);
      // Dispatch EVRF macro-ops
      dispatch_logic(evrf_dispatch_flag[sector_id], evrf_pc[sector_id], evrf_mop_fifo_almost_full[sector_id],
                     evrf_mop_fifo_wen[sector_id], inst_memory[evrf_pc[sector_id].read()].evrf_inst.op);
      evrf_mop_fifo_wdata[sector_id].write(inst_memory[evrf_pc[sector_id].read()].evrf_inst);
      // Dispatch MFU0 macro-ops
      dispatch_logic(mfu0_dispatch_flag[sector_id], mfu0_pc[sector_id], mfu0_mop_fifo_almost_full[sector_id],
                     mfu0_mop_fifo_wen[sector_id], inst_memory[mfu0_pc[sector_id].read()].mfu0_inst.op.to_uint() != 0);
      mfu0_mop_fifo_wdata[sector_id].write(inst_memory[mfu0_pc[sector_id].read()].mfu0_inst);
      // Dispatch MFU1 macro-ops
      dispatch_logic(mfu1_dispatch_flag[sector_id], mfu1_pc[sector_id], mfu1_mop_fifo_almost_full[sector_id],
                     mfu1_mop_fifo_wen[sector_id], inst_memory[mfu1_pc[sector_id].read()].mfu1_inst.op.to_uint() != 0);
      mfu1_mop_fifo_wdata[sector_id].write(inst_memory[mfu1_pc[sector_id].read()].mfu1_inst);
    }
    // Dispatch Loader macro-ops
    dispatch_logic(ld_dispatch_flag, ld_pc, ld_mop_fifo_almost_full, ld_mop_fifo_wen, inst_memory[ld_pc.read()].ld_inst.op);
    ld_mop_fifo_wdata.write(inst_memory[ld_pc.read()].ld_inst);
    wait();
  }
}

void vliw_inst_dispatch::Assign() {
  // Set the FIFO ready signals of all macro-op FIFOs of different NPU blocks
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    sector_mop_rdy[sector_id][0].write(!sector_mop_fifo_empty[sector_id].read());
    evrf_mop_rdy[sector_id][0].write(!evrf_mop_fifo_empty[sector_id].read());
    mfu0_mop_rdy[sector_id][0].write(!mfu0_mop_fifo_empty[sector_id].read());
    mfu1_mop_rdy[sector_id][0].write(!mfu1_mop_fifo_empty[sector_id].read());
  }
  ld_mop_rdy[0].write(!ld_mop_fifo_empty.read());
}

sector_and_accum_decoder::sector_and_accum_decoder(const sc_module_name& name) : sc_module(name) {
  mop_fifo = new instruction_fifo<mvu_mop>("mop_fifo", MOP_FIFO_DEPTH, MOP_FIFO_ALMOST_FULL_DEPTH, 0);
  mop_fifo->clk(clk);
  mop_fifo->rst(rst);
  mop_fifo->wen(mop_fifo_wen);
  mop_fifo->wdata(mop_rdata);
  mop_fifo->ren(mop_fifo_ren);
  mop_fifo->rdata(mop_fifo_rdata);
  mop_fifo->full(mop_fifo_full);
  mop_fifo->almost_full(mop_fifo_almost_full);
  mop_fifo->empty(mop_fifo_empty);
  mop_fifo->almost_empty(mop_fifo_almost_empty);

  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
  SC_METHOD(Assign);
  sensitive << mop_rdy << mop_fifo_almost_full;
}

sector_and_accum_decoder::~sector_and_accum_decoder() {}

void sector_and_accum_decoder::Assign() {
  if (rst.read()) {
    mop_fifo_wen.write(false);
    mop_ren.write(false);
  } else {
    mop_fifo_wen.write(!mop_fifo_almost_full.read() && mop_rdy.read());
    mop_ren.write(!mop_fifo_almost_full.read() && mop_rdy.read());
  }
}

void sector_and_accum_decoder::Tick() {
  // Reset logic
  decode_state.write(DEC_MVU_READ_INST);
  batch_counter.write(0);
  vrf_counter.write(0);
  pipeline_counter.write(0);
  vrf_id_counter.write(0);
  vrf_id.write(0);
  mrf_chunk_offset.write(0);
  mrf_pipeline_offset.write(0);
  mrf_addr.write(0);
  mvu_tag.write(0);
  reg_sel.write(false);
  remaining_words.write(0);
  acc_size.write(0);
  wait();

  // Sequential logic
  while (true) {
    decoded_mvu_uop = mvu_uop();
    uop_wen.write(false);

    switch (decode_state) {
      // If macro-op is ready, read it and move to the initialization decoding stage
      case DEC_MVU_READ_INST:
        if (!mop_fifo_empty.read()) {
          nxt_mvu_inst.write(mop_fifo_rdata);
          decode_state.write(DEC_MVU_INIT);
          mop_fifo_ren.write(true);
          first_uop = true;
          // cout << "Tile got " << mop_rdata << endl;
        }
        break;

      // If macro-op has valid operation (not no-op), initialize signals/counters and move to core decoding stage.
      // Else, ignore and move to macro-op reading stage.
      case DEC_MVU_INIT:
        mop_fifo_ren.write(false);
        if (nxt_mvu_inst.read().op) {
          decode_state.write(DEC_MVU_CORE);
          batch_counter.write(0);
          vrf_counter.write(0);
          pipeline_counter.write(0);
          vrf_id_counter.write(0);
          vrf_id.write(0);
          mrf_chunk_offset.write(0);
          mrf_pipeline_offset.write(0);
          mrf_addr.write(nxt_mvu_inst.read().mrf_base);
          mvu_tag.write(nxt_mvu_inst.read().tag);
          reg_sel.write(reg_sel.read());
          remaining_words.write(nxt_mvu_inst.read().words_per_row);
          acc_size.write((DPE_NUM_TBS - 1) * TB_NUM_DOTS);
        } else {
          decode_state.write(DEC_MVU_READ_INST);
        }
        break;

      // Stay in core decoding stage for as many cycles as needed to decode the macro-op in hand into a sequence of
      // micro-ops, then move back to macro-op reading stage
      case DEC_MVU_CORE:
        // Set micro-op fields
        for (unsigned int i = 0; i < TB_NUM_DOTS; i++)
          if (batch_counter.read() == i)
            decoded_mvu_uop.vrf_addr = nxt_mvu_inst.read().vrf_base[i] + vrf_counter.read();

        decoded_mvu_uop.vrf_rd_id = vrf_id.read();
        decoded_mvu_uop.reg_sel = reg_sel.read();
        decoded_mvu_uop.mrf_addr = mrf_addr.read();
        decoded_mvu_uop.tag = mvu_tag.read();

        if (pipeline_counter.read() < (DPE_NUM_TBS - 1) * TB_NUM_DOTS)
          decoded_mvu_uop.vrf_en = true;
        else
          decoded_mvu_uop.vrf_en = false;

        if (remaining_words.read() >= (2 * (DPE_NUM_TBS - 1) * TB_NUM_DOTS)) {
          decoded_mvu_uop.accum_size = (DPE_NUM_TBS - 1) * TB_NUM_DOTS;
          acc_size.write((DPE_NUM_TBS - 1) * TB_NUM_DOTS);
        } else {
          decoded_mvu_uop.accum_size = remaining_words.read();
          acc_size.write(remaining_words.read());
        }

        if (pipeline_counter.read() < remaining_words.read()) {
          if ((vrf_counter.read() == 0) && (vrf_counter.read() == nxt_mvu_inst.read().vrf_size - 1))
            decoded_mvu_uop.accum_op = ACCUM_OP_SWB;
          else if (vrf_counter.read() == nxt_mvu_inst.read().vrf_size - 1)
            decoded_mvu_uop.accum_op = ACCUM_OP_WRB;
          else if (vrf_counter.read() == 0)
            decoded_mvu_uop.accum_op = ACCUM_OP_SET;
          else
            decoded_mvu_uop.accum_op = ACCUM_OP_UPD;
        } else {
          decoded_mvu_uop.accum_op = ACCUM_OP_SET;
        }

        // Update decoding signals/counters if micro-op FIFO is ready to accept the decoded micro-op
        if (uop_rdy.read()) {
          if (pipeline_counter.read() < (DPE_NUM_TBS - 1) * TB_NUM_DOTS) {
            if (batch_counter.read() == TB_NUM_DOTS - 1)
              batch_counter.write(0);
            else
              batch_counter.write(batch_counter.read() + 1);

            if (vrf_id_counter.read() == TB_NUM_DOTS - 1)
              vrf_id_counter.write(0);
            else
              vrf_id_counter.write(vrf_id_counter.read() + 1);

            if ((vrf_id.read() == DPE_NUM_TBS - 2) && (vrf_id_counter.read() == TB_NUM_DOTS - 1))
              vrf_id.write(0);
            else if (vrf_id_counter.read() == TB_NUM_DOTS - 1)
              vrf_id.write(vrf_id.read() + 1);
            else
              vrf_id.write(vrf_id.read());
          }

          sc_uint<VRF_ADDRW> vrf_counter_value;
          if (pipeline_counter.read() == acc_size.read() - 1) {
            if (vrf_counter.read() == nxt_mvu_inst.read().vrf_size - 1)
              vrf_counter_value = 0;
            else
              vrf_counter_value = vrf_counter.read() + 1;
          } else {
            vrf_counter_value = vrf_counter.read();
          }
          vrf_counter.write(vrf_counter_value);

          if (pipeline_counter.read() == acc_size.read() - 1)
            pipeline_counter.write(0);
          else
            pipeline_counter.write(pipeline_counter.read() + 1);

          if ((pipeline_counter.read() == acc_size.read() - 1) ||
              (mrf_addr.read() == (nxt_mvu_inst.read().mrf_base + nxt_mvu_inst.read().mrf_size - 1))) {
            reg_sel.write(!reg_sel.read());
          } else {
            reg_sel.write(reg_sel.read());
          }

          sc_uint<MRF_ADDRW> mrf_chunk_offset_value;
          if ((vrf_counter.read() == nxt_mvu_inst.read().vrf_size - 1) &&
              (pipeline_counter.read() == acc_size.read() - 1)) {
            mrf_chunk_offset_value =
                mrf_chunk_offset.read() + ((DPE_NUM_TBS - 1) * TB_NUM_DOTS * nxt_mvu_inst.read().vrf_size);
          } else {
            mrf_chunk_offset_value = mrf_chunk_offset.read();
          }
          mrf_chunk_offset.write(mrf_chunk_offset_value);

          sc_uint<MRF_ADDRW> mrf_pipeline_offset_value;
          if (pipeline_counter.read() == acc_size.read() - 1)
            mrf_pipeline_offset_value = 0;
          else
            mrf_pipeline_offset_value = mrf_pipeline_offset.read() + nxt_mvu_inst.read().vrf_size;
          mrf_pipeline_offset.write(mrf_pipeline_offset_value);

          mrf_addr.write(nxt_mvu_inst.read().mrf_base + mrf_chunk_offset_value + mrf_pipeline_offset_value +
                         vrf_counter_value);

          if (vrf_counter.read() == nxt_mvu_inst.read().vrf_size - 1 &&
              (pipeline_counter.read() == acc_size.read() - 1) &&
              (remaining_words.read() > (2 * (DPE_NUM_TBS - 1) * TB_NUM_DOTS - 1))) {
            remaining_words.write(remaining_words.read() - ((DPE_NUM_TBS - 1) * TB_NUM_DOTS));
          } else {
            remaining_words.write(remaining_words.read());
          }

          if (mrf_addr.read() == (nxt_mvu_inst.read().mrf_base + nxt_mvu_inst.read().mrf_size - 1)) {
            decode_state.write(DEC_MVU_READ_INST);
            decoded_mvu_uop.last_uop = true;
          }

          decoded_mvu_uop.first_uop = first_uop;
          first_uop = false;
          uop_wen.write(true);
          uop_wdata.write(decoded_mvu_uop);

          // Log decoded micro-op
          std::stringstream uop_str;
          uop_str << decoded_mvu_uop;
          sim_log.log(info, uop_str.str(), this->name());
        }
        break;

      default:
        break;
    }

    wait();
  }
}

evrf_decoder::evrf_decoder(const sc_module_name& name) : sc_module(name) {
  mop_fifo = new instruction_fifo<evrf_mop>("mop_fifo", MOP_FIFO_DEPTH, MOP_FIFO_ALMOST_FULL_DEPTH, 0);
  mop_fifo->clk(clk);
  mop_fifo->rst(rst);
  mop_fifo->wen(mop_fifo_wen);
  mop_fifo->wdata(mop_rdata);
  mop_fifo->ren(mop_fifo_ren);
  mop_fifo->rdata(mop_fifo_rdata);
  mop_fifo->full(mop_fifo_full);
  mop_fifo->almost_full(mop_fifo_almost_full);
  mop_fifo->empty(mop_fifo_empty);
  mop_fifo->almost_empty(mop_fifo_almost_empty);

  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
  SC_METHOD(Assign);
  sensitive << mop_rdy << mop_fifo_almost_full;
}

evrf_decoder::~evrf_decoder() {}

void evrf_decoder::Assign() {
  if (rst.read()) {
    mop_fifo_wen.write(false);
    mop_ren.write(false);
  } else {
    mop_fifo_wen.write(!mop_fifo_almost_full.read() && mop_rdy.read());
    mop_ren.write(!mop_fifo_almost_full.read() && mop_rdy.read());
  }
}

void evrf_decoder::Tick() {
  // Reset logic
  decode_state.write(DEC_EVRF_READ_INST);
  evrf_batch_counter.write(0);
  evrf_addr_counter.write(0);
  wait();

  // Sequential logic
  while (true) {
    decoded_evrf_uop = evrf_uop();
    uop_wen.write(false);

    switch (decode_state) {
      // If macro-op is ready, read it and move to the initialization decoding stage
      case DEC_EVRF_READ_INST:
        if (!mop_fifo_empty.read()) {
          nxt_evrf_inst.write(mop_fifo_rdata);
          decode_state.write(DEC_EVRF_INIT);
          mop_fifo_ren.write(true);
          first_uop = true;
          // cout << "eVRF got " << mop_rdata << endl;
        }
        break;

      // If macro-op has valid operation (not no-op), initialize signals/counters and move to core decoding stage.
      // Else, ignore and move to macro-op reading stage.
      case DEC_EVRF_INIT:
        mop_fifo_ren.write(false);
        if (nxt_evrf_inst.read().op) {
          decode_state.write(DEC_EVRF_CORE);
          evrf_batch_counter.write(0);
          evrf_addr_counter.write(0);
        } else {
          decode_state.write(DEC_EVRF_READ_INST);
        }
        break;

      // Stay in core decoding stage for as many cycles as needed to decode the macro-op in hand into a sequence of
      // micro-ops, then move back to macro-op reading stage
      case DEC_EVRF_CORE:
        // Set micro-op fields
        for (unsigned int i = 0; i < TB_NUM_DOTS; i++)
          if (evrf_batch_counter.read() == i)
            decoded_evrf_uop.vrf_addr = nxt_evrf_inst.read().vrf_base[i] + evrf_addr_counter.read();

        if (evrf_batch_counter.read() < nxt_evrf_inst.read().batch) {
          decoded_evrf_uop.src_sel = nxt_evrf_inst.read().src_sel;
        } else {
          decoded_evrf_uop.src_sel = EVRF_FLUSH_MVU;
        }
        decoded_evrf_uop.tag = nxt_evrf_inst.read().tag;

        // Update decoding signals/counters
        if (uop_rdy.read()) {
          if ((evrf_batch_counter.read() == TB_NUM_DOTS - 1) && (nxt_evrf_inst.read().src_sel == EVRF_FROM_MVU)) {
            evrf_batch_counter.write(0);
          } else if ((evrf_batch_counter.read() == nxt_evrf_inst.read().batch - 1) &&
                     (nxt_evrf_inst.read().src_sel == EVRF_FROM_VRF)) {
            evrf_batch_counter.write(0);
          } else {
            evrf_batch_counter.write(evrf_batch_counter.read() + 1);
          }

          if ((evrf_batch_counter.read() == TB_NUM_DOTS - 1) && (nxt_evrf_inst.read().src_sel == EVRF_FROM_MVU)) {
            evrf_addr_counter.write(evrf_addr_counter.read() + 1);
          } else if ((evrf_batch_counter.read() == nxt_evrf_inst.read().batch - 1) &&
                     (nxt_evrf_inst.read().src_sel == EVRF_FROM_VRF)) {
            evrf_addr_counter.write(evrf_addr_counter.read() + 1);
          } else {
            evrf_addr_counter.write(evrf_addr_counter.read());
          }

          if ((evrf_addr_counter.read() == nxt_evrf_inst.read().vrf_size - 1) &&
              (((evrf_batch_counter.read() == TB_NUM_DOTS - 1) && (nxt_evrf_inst.read().src_sel == EVRF_FROM_MVU)) ||
               ((evrf_batch_counter.read() == nxt_evrf_inst.read().batch - 1) &&
                (nxt_evrf_inst.read().src_sel == EVRF_FROM_VRF)))) {
            decode_state.write(DEC_EVRF_READ_INST);
            decoded_evrf_uop.last_uop = true;
          }

          decoded_evrf_uop.first_uop = first_uop;
          first_uop = false;
          uop_wen.write(true);
          uop_wdata.write(decoded_evrf_uop);

          // Log decoded micro-op
          std::stringstream uop_str;
          uop_str << decoded_evrf_uop;
          sim_log.log(info, uop_str.str(), this->name());
        }
        break;

      default:
        break;
    }
    wait();
  }
}

mfu_decoder::mfu_decoder(const sc_module_name& name) : sc_module(name) {
  mop_fifo = new instruction_fifo<mfu_mop>("mop_fifo", MOP_FIFO_DEPTH, MOP_FIFO_ALMOST_FULL_DEPTH, 0);
  mop_fifo->clk(clk);
  mop_fifo->rst(rst);
  mop_fifo->wen(mop_fifo_wen);
  mop_fifo->wdata(mop_rdata);
  mop_fifo->ren(mop_fifo_ren);
  mop_fifo->rdata(mop_fifo_rdata);
  mop_fifo->full(mop_fifo_full);
  mop_fifo->almost_full(mop_fifo_almost_full);
  mop_fifo->empty(mop_fifo_empty);
  mop_fifo->almost_empty(mop_fifo_almost_empty);

  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
  SC_METHOD(Assign);
  sensitive << mop_rdy << mop_fifo_almost_full;
}

mfu_decoder::~mfu_decoder() {}

void mfu_decoder::Assign() {
  if (rst.read()) {
    mop_fifo_wen.write(false);
    mop_ren.write(false);
  } else {
    mop_fifo_wen.write(!mop_fifo_almost_full.read() && mop_rdy.read());
    mop_ren.write(!mop_fifo_almost_full.read() && mop_rdy.read());
  }
}

void mfu_decoder::Tick() {
  // Reset logic
  decode_state.write(DEC_MFU_READ_INST);
  mfu_batch_counter.write(0);
  mfu_counter.write(0);
  mfu_size.write(0);
  mfu_tag.write(0);
  mfu_op.write(0);
  mfu_batch.write(0);
  wait();

  // Sequential logic
  while (true) {
    decoded_mfu_uop = mfu_uop();
    uop_wen.write(false);

    switch (decode_state) {
      // If macro-op is ready, read it and move to the initialization decoding stage
      case DEC_MFU_READ_INST:
        if (!mop_fifo_empty.read()) {
          nxt_mfu_inst.write(mop_fifo_rdata);
          decode_state.write(DEC_MFU_INIT);
          mop_fifo_ren.write(true);
          first_uop = true;
          // cout << "MFU got " << mop_rdata << endl;
        }
        break;

      // If macro-op has valid operation (not no-op), initialize signals/counters and move to core decoding stage.
      // Else, ignore and move to macro-op reading stage.
      case DEC_MFU_INIT:
        mop_fifo_ren.write(false);
        if ((nxt_mfu_inst.read().op >> 6) & 0x1) {
          mfu_batch_counter.write(0);
          mfu_counter.write(0);
          mfu_size.write(nxt_mfu_inst.read().vrf_size);
          mfu_tag.write(nxt_mfu_inst.read().tag);
          mfu_op.write(nxt_mfu_inst.read().op);
          mfu_batch.write(nxt_mfu_inst.read().batch);
          mfu_counter.write(0);
          mfu_batch_counter.write(0);
          decode_state.write(DEC_MFU_CORE);
        } else {
          decode_state.write(DEC_MFU_READ_INST);
        }
        break;

      // Stay in core decoding stage for as many cycles as needed to decode the macro-op in hand into a sequence of
      // micro-ops, then move back to macro-op reading stage
      case DEC_MFU_CORE:
        // Set micro-op fields
        for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
          if (mfu_batch_counter.read() == i) {
            decoded_mfu_uop.vrf0_addr = nxt_mfu_inst.read().add_vrf_base[i] + mfu_counter.read();
            decoded_mfu_uop.vrf1_addr = nxt_mfu_inst.read().mul_vrf_base[i] + mfu_counter.read();
          }
        }
        decoded_mfu_uop.tag = mfu_tag.read();
        decoded_mfu_uop.activation_op = (mfu_op.read() >> 4) & 0x3;
        decoded_mfu_uop.add_op = (mfu_op.read() >> 1) & 0x7;
        decoded_mfu_uop.mult_op = mfu_op.read() & 0x1;

        // Update decoding signals/counters
        if (uop_rdy.read()) {
          if (mfu_batch_counter.read() == mfu_batch.read() - 1) {
            mfu_counter.write(mfu_counter.read() + 1);
            mfu_batch_counter.write(0);
          } else {
            mfu_counter.write(mfu_counter.read());
            mfu_batch_counter.write(mfu_batch_counter.read() + 1);
          }

          if ((mfu_counter.read() == mfu_size.read() - 1) && (mfu_batch_counter.read() == mfu_batch.read() - 1)) {
            decode_state.write(DEC_MFU_READ_INST);
            decoded_mfu_uop.last_uop = true;
          }

          decoded_mfu_uop.first_uop = first_uop;
          first_uop = false;
          uop_wen.write(true);
          uop_wdata.write(decoded_mfu_uop);

          // Log decoded micro-op
          std::stringstream uop_str;
          uop_str << decoded_mfu_uop;
          sim_log.log(info, uop_str.str(), this->name());
        }
        break;

      default:
        break;
    }
    wait();
  }
}

ld_decoder::ld_decoder(const sc_module_name& name) : sc_module(name) {
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

ld_decoder::~ld_decoder() {}

void ld_decoder::Tick() {
  // Reset logic
  decode_state.write(DEC_LD_READ_INST);
  ld_batch_counter.write(0);
  ld_batch.write(0);
  ld_counter.write(0);
  ld_size.write(0);
  ld_src_sel.write(0);
  ld_interrupt.write(false);
  ld_report_to_host.write(false);
  wait();

  // Sequential logic
  while (true) {
    decoded_ld_uop = ld_uop();
    uop_wen.write(false);

    switch (decode_state) {
      // If macro-op is ready, read it and move to the initialization decoding stage
      case DEC_LD_READ_INST:
        if (mop_rdy.read()) {
          nxt_ld_inst.write(mop_rdata);
          decode_state.write(DEC_LD_INIT);
          mop_ren.write(true);
          first_uop = true;
          // cout << "Loader got " << mop_rdata << endl;
        }
        break;

      // If macro-op has valid operation (not no-op), initialize signals/counters and move to core decoding stage.
      // Else, ignore and move to macro-op reading stage.
      case DEC_LD_INIT:
        mop_ren.write(false);
        if (nxt_ld_inst.read().op) {
          ld_size.write(nxt_ld_inst.read().vrf_size);
          ld_src_sel.write(nxt_ld_inst.read().src_sel);
          ld_interrupt.write(nxt_ld_inst.read().interrupt);
          ld_report_to_host.write(nxt_ld_inst.read().output_result);
          ld_batch.write(nxt_ld_inst.read().batch);
          ld_counter.write(0);
          ld_batch_counter.write(0);
          ld_tag_counter.write(0);
          decode_state.write(DEC_LD_CORE);
        } else {
          decode_state.write(DEC_LD_READ_INST);
        }
        break;

      // Stay in core decoding stage for as many cycles as needed to decode the macro-op in hand into a sequence of
      // micro-ops, then move to tag update stage
      case DEC_LD_CORE:
        // Set micro-op fields
        decoded_ld_uop.vrf0_id = nxt_ld_inst.read().vrf0_id;
        decoded_ld_uop.block1_id = nxt_ld_inst.read().block1_id;
        decoded_ld_uop.vrf1_id = nxt_ld_inst.read().vrf1_id;
        for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
          if (ld_batch_counter.read() == i) {
            decoded_ld_uop.vrf0_addr = nxt_ld_inst.read().vrf0_base[i] + ld_counter.read();
            decoded_ld_uop.vrf1_addr = nxt_ld_inst.read().vrf1_base[i] + ld_counter.read();
          }
        }
        decoded_ld_uop.src_sel = ld_src_sel.read();
        decoded_ld_uop.last = false;
        decoded_ld_uop.interrupt = false;
        decoded_ld_uop.output_result = ld_report_to_host.read();

        // Update decoding signals/counters
        if (uop_rdy.read()) {
          decoded_ld_uop.first_uop = first_uop;
          first_uop = false;
          uop_wen.write(true);
          uop_wdata.write(decoded_ld_uop);

          if (ld_batch_counter.read() == ld_batch.read() - 1) {
            ld_batch_counter.write(0);
            ld_counter.write(ld_counter.read() + 1);
          } else {
            ld_batch_counter.write(ld_batch_counter.read() + 1);
            ld_counter.write(ld_counter.read());
          }

          if ((ld_counter.read() == ld_size.read() - 1) && (ld_batch_counter.read() == ld_batch.read() - 1)) {
            decode_state.write(DEC_LD_TAG_UPDATE);
          }

          // Log decoded micro-op
          std::stringstream uop_str;
          uop_str << decoded_ld_uop;
          sim_log.log(info, uop_str.str(), this->name());
        }
        break;

      // Issue tag update micro-ops to all NPU blocks, then move back to macro-op reading stage
      case DEC_LD_TAG_UPDATE:
        decoded_ld_uop.vrf0_id = 0;
        decoded_ld_uop.block1_id = 0;
        decoded_ld_uop.vrf1_id = 0;
        decoded_ld_uop.vrf0_addr = VRF_DEPTH - 1;
        decoded_ld_uop.vrf1_addr = VRF_DEPTH - 1;
        decoded_ld_uop.src_sel = LD_TAG_UPDATE;
        decoded_ld_uop.last = true;
        decoded_ld_uop.interrupt = false;
        decoded_ld_uop.output_result = false;

        if (uop_rdy.read()) {
          if (ld_tag_counter.read() == 0) {
            decoded_ld_uop.vrf0_id = 1;
            decoded_ld_uop.block1_id = 0;
            decoded_ld_uop.vrf1_id = 0;
            ld_tag_counter.write(ld_tag_counter.read() + 1);
          } else if (ld_tag_counter.read() < NUM_PIPELINE_BLOCKS - 2) {
            decoded_ld_uop.vrf0_id = 0;
            decoded_ld_uop.block1_id = ld_tag_counter.read();
            decoded_ld_uop.vrf1_id = 1;
            ld_tag_counter.write(ld_tag_counter.read() + 1);
          } else {
            decoded_ld_uop.vrf0_id = 0;
            decoded_ld_uop.block1_id = ld_tag_counter.read();
            decoded_ld_uop.vrf1_id = 1;
            decoded_ld_uop.interrupt = ld_interrupt.read();
            decoded_ld_uop.last_uop = true;
            decode_state.write(DEC_LD_READ_INST);
          }
          uop_wen.write(true);
          uop_wdata.write(decoded_ld_uop);

          // Log decoded micro-op
          std::stringstream uop_str;
          uop_str << decoded_ld_uop;
          sim_log.log(info, uop_str.str(), this->name());
        }
        break;

      default:
        break;
    }
    wait();
  }
}