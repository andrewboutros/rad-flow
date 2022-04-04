#include "axis_mvu_sector_chain.hpp"

axis_mvu_sector_chain::axis_mvu_sector_chain(const sc_module_name& name, unsigned int id, unsigned int inst_interface_dataw,
                                 unsigned int wb_interface_dataw, unsigned int ofifo_interface_dataw,
                                 unsigned int chain_ofifo_interface_dataw)
    : sc_module(name),
      mrf_waddr("mrf_waddr"),
      mrf_wdata("mrf_wdata"),
      mrf_wid("mrf_wid"),
      sector_inst_interface("sector_inst_interface", THREADS),
      sector_chain_ofifo_rdy("sector_chain_ofifo_rdy"),
      sector_chain_ofifo_ren("sector_chain_ofifo_ren"),
      sector_chain_ofifo_rdata("sector_chain_ofifo_rdata"),
      sector_ofifo_interface("sector_ofifo_interface", THREADS),
      ofifo_rdy_signal("ofifo_rdy_signal"),
      ofifo_ren_signal("ofifo_ren_signal"),
      ofifo_rdata_signal("ofifo_rdata_signal"),
      uop_rdy_signal("uop_rdy_signal", THREADS),
      uop_wen_signal("uop_wen_signal", THREADS),
      uop_wdata_signal("uop_wdata_signal", THREADS),
      mop_rdy_signal("mop_rdy_signal"),
      mop_ren_signal("mop_ren_signal"),
      mop_rdata_signal("mop_rdata_signal") {
  sector_id = id;

  init_vector<sc_signal<bool>>::init_sc_vector(ofifo_rdy_signal, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ofifo_ren_signal, THREADS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(ofifo_rdata_signal, THREADS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(sector_chain_ofifo_rdy, THREADS, CORES);
  init_vector<sc_in<bool>>::init_sc_vector(sector_chain_ofifo_ren, THREADS, CORES);
  init_vector<sc_out<data_vector<tb_input_precision>>>::init_sc_vector(sector_chain_ofifo_rdata, THREADS, CORES);
  init_vector<sc_in<bool>>::init_sc_vector(wb_fifo_rdy, THREADS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(wb_fifo_ren, THREADS, CORES);
  init_vector<sc_in<data_vector<tb_input_precision>>>::init_sc_vector(wb_fifo_rdata, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(mop_rdy_signal, THREADS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(mop_ren_signal, THREADS, 1);
  init_vector<sc_signal<mvu_mop>>::init_sc_vector(mop_rdata_signal, THREADS, 1);

  char sector_decoder_name[NAME_LENGTH], inst_interface_name[NAME_LENGTH], wb_interface_name[NAME_LENGTH],
      sector_ofifo_interface_name[NAME_LENGTH], sector_chain_ofifo_interface_name[NAME_LENGTH];
  std::string sector_decoder_name_str, inst_interface_name_str, wb_interface_name_str, sector_ofifo_interface_name_str,
      sector_chain_ofifo_interface_name_str;
  sector_decoder.resize(THREADS);
  inst_axis_interface.resize(THREADS);
  ofifo_axis_interface.resize(THREADS);

  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    // Create sector instruction decoder module
    sector_decoder_name_str = "sector" + std::to_string(sector_id) + "_decoder_" + std::to_string(thread_id);
    std::strcpy(sector_decoder_name, sector_decoder_name_str.c_str());
    sector_decoder[thread_id] = new sector_and_accum_decoder(sector_decoder_name);
    sector_decoder[thread_id]->clk(clk);
    sector_decoder[thread_id]->rst(rst);
    sector_decoder[thread_id]->mop_rdy(mop_rdy_signal[thread_id][0]);
    sector_decoder[thread_id]->mop_ren(mop_ren_signal[thread_id][0]);
    sector_decoder[thread_id]->mop_rdata(mop_rdata_signal[thread_id][0]);
    sector_decoder[thread_id]->uop_rdy(uop_rdy_signal[thread_id]);
    sector_decoder[thread_id]->uop_wen(uop_wen_signal[thread_id]);
    sector_decoder[thread_id]->uop_wdata(uop_wdata_signal[thread_id]);

    // Create the instruction slave AXI-streaming interface (receives macro-ops from top-level instruction dispatcher)
    inst_interface_name_str = "sector" + std::to_string(sector_id) + "_inst_interface_" + std::to_string(thread_id);
    std::strcpy(inst_interface_name, inst_interface_name_str.c_str());
    inst_axis_interface[thread_id] = new axis_slave_fifo_adapter<mvu_mop, sc_bv<MVU_MOP_BITWIDTH>>(
        inst_interface_name, inst, inst_interface_dataw, 1, MVU_MOP_BITWIDTH, 1);
    inst_axis_interface[thread_id]->clk(clk);
    inst_axis_interface[thread_id]->rst(rst);
    inst_axis_interface[thread_id]->fifo_rdy(mop_rdy_signal[thread_id]);
    inst_axis_interface[thread_id]->fifo_ren(mop_ren_signal[thread_id]);
    inst_axis_interface[thread_id]->fifo_rdata(mop_rdata_signal[thread_id]);
    inst_axis_interface[thread_id]->axis_port.ConnectToPort(sector_inst_interface[thread_id]);

    unsigned int ofifo_dest, chain_ofifo_dest;
#ifdef RADSIM
    std::string dest_name = "axis_vector_elementwise_" + std::to_string(thread_id) + "_" + std::to_string(sector_id);
    ofifo_dest = module_assignment[dest_name];
    if (sector_id < SECTORS - 1) {
      dest_name = "axis_mvu_sector_" + std::to_string(sector_id + 1);
      chain_ofifo_dest = module_assignment[dest_name];
    } else {
      chain_ofifo_dest = NOC_NODES - 1;
    }
#else
    ofifo_dest = (sector_id * 6) + EVRF_DEST_ID;
    chain_ofifo_dest = (sector_id + 1) * 6;
#endif

    // Create the output master AXI-streaming interface (send data to downstream module)
    sector_ofifo_interface_name_str =
        "sector" + std::to_string(sector_id) + "_ofifo_interface_" + std::to_string(thread_id);
    std::strcpy(sector_ofifo_interface_name, sector_ofifo_interface_name_str.c_str());
    ofifo_axis_interface[thread_id] =
        new axis_master_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>(
            sector_ofifo_interface_name, feedforward, ofifo_interface_dataw, CORES, HIGH_PRECISION, ofifo_dest,
            EVRF_IN_STREAM_ID);
    ofifo_axis_interface[thread_id]->clk(clk);
    ofifo_axis_interface[thread_id]->rst(rst);
    ofifo_axis_interface[thread_id]->fifo_rdy(ofifo_rdy_signal[thread_id]);
    ofifo_axis_interface[thread_id]->fifo_ren(ofifo_ren_signal[thread_id]);
    ofifo_axis_interface[thread_id]->fifo_rdata(ofifo_rdata_signal[thread_id]);
    ofifo_axis_interface[thread_id]->axis_port.ConnectToPort(sector_ofifo_interface[thread_id]);
  }

  // Create sector module
  char sector_name[NAME_LENGTH];
  std::string sector_name_str = "sector" + std::to_string(sector_id);
  std::strcpy(sector_name, sector_name_str.c_str());
  sector_module = new mvu_sector(sector_name, sector_id);
  sector_module->clk(clk);
  sector_module->rst(rst);
  sector_module->inst(uop_wdata_signal);
  sector_module->inst_fifo_wen(uop_wen_signal);
  sector_module->inst_fifo_rdy(uop_rdy_signal);
  sector_module->wb_fifo_rdy(wb_fifo_rdy);
  sector_module->wb_fifo_ren(wb_fifo_ren);
  sector_module->wb_fifo_rdata(wb_fifo_rdata);
  sector_module->mrf_waddr(mrf_waddr);
  sector_module->mrf_wdata(mrf_wdata);
  sector_module->mrf_wid(mrf_wid);
  sector_module->sector_chain_ofifo_rdy(sector_chain_ofifo_rdy);
  sector_module->sector_chain_ofifo_ren(sector_chain_ofifo_ren);
  sector_module->sector_chain_ofifo_rdata(sector_chain_ofifo_rdata);
  sector_module->sector_ofifo_rdy(ofifo_rdy_signal);
  sector_module->sector_ofifo_ren(ofifo_ren_signal);
  sector_module->sector_ofifo_rdata(ofifo_rdata_signal);
}

axis_mvu_sector_chain::~axis_mvu_sector_chain() {
  delete sector_module;
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    delete inst_axis_interface[thread_id];
    delete ofifo_axis_interface[thread_id];
  }
}