#include <axis_evrf.hpp>

axis_evrf::axis_evrf(const sc_module_name& name, unsigned int id, unsigned int id_thread, unsigned int inst_interface_dataw,
            unsigned int wb_interface_dataw, unsigned int ififo_interface_dataw, unsigned int ofifo_interface_dataw)
    : sc_module(name),
      ififo_rdy_signal("ififo_rdy_signal", CORES),
      ififo_ren_signal("ififo_ren_signal", CORES),
      ififo_rdata_signal("ififo_rdata_signal", CORES),
      ofifo_rdy_signal("ofifo_rdy_signal", CORES),
      ofifo_ren_signal("ofifo_ren_signal", CORES),
      ofifo_rdata_signal("ofifo_rdata_signal", CORES),
      wb_rdy_signal("wb_rdy_signal", CORES),
      wb_ren_signal("wb_ren_signal", CORES),
      wb_rdata_signal("wb_rdata_signal", CORES),
      uop_rdy_signal("uop_rdy_signal"),
      uop_wen_signal("uop_wen_signal"),
      uop_wdata_signal("uop_wdata_signal"),
      mop_rdy_signal("mop_rdy_signal", 1),
      mop_ren_signal("mop_ren_signal", 1),
      mop_rdata_signal("mop_rdata_signal", 1) {
  sector_id = id;
  thread_id = id_thread;
  
  // Create external VRF instruction decoder module
  decoder = new evrf_decoder("evrf_decoder");
  decoder->clk(clk);
  decoder->rst(rst);
  decoder->mop_rdy(mop_rdy_signal[0]);
  decoder->mop_ren(mop_ren_signal[0]);
  decoder->mop_rdata(mop_rdata_signal[0]);
  decoder->uop_rdy(uop_rdy_signal);
  decoder->uop_wen(uop_wen_signal);
  decoder->uop_wdata(uop_wdata_signal);

  // Create external VRF module
  evrf_module = new evrf("evrf", sector_id, thread_id);
  evrf_module->clk(clk);
  evrf_module->rst(rst);
  evrf_module->inst(uop_wdata_signal);
  evrf_module->inst_fifo_wen(uop_wen_signal);
  evrf_module->inst_fifo_rdy(uop_rdy_signal);
  evrf_module->wbfifo_rdy(wb_rdy_signal);
  evrf_module->wbfifo_ren(wb_ren_signal);
  evrf_module->wbfifo_rdata(wb_rdata_signal);
  evrf_module->mvu_ofifo_rdy(ififo_rdy_signal);
  evrf_module->mvu_ofifo_ren(ififo_ren_signal);
  evrf_module->mvu_ofifo_rdata(ififo_rdata_signal);
  evrf_module->evrf_ofifo_rdy(ofifo_rdy_signal);
  evrf_module->evrf_ofifo_ren(ofifo_ren_signal);
  evrf_module->evrf_result(ofifo_rdata_signal);

  // Create the instruction slave AXI-streaming interface (receives macro-ops from top-level instruction dispatcher)
  inst_axis_interface = new axis_slave_fifo_adapter<evrf_mop, sc_bv<EVRF_MOP_BITWIDTH>>("evrf_inst_axis_interface",
                                                                                        inst, inst_interface_dataw, 1, EVRF_MOP_BITWIDTH, 1);
  inst_axis_interface->clk(clk);
  inst_axis_interface->rst(rst);
  inst_axis_interface->fifo_rdy(mop_rdy_signal);
  inst_axis_interface->fifo_ren(mop_ren_signal);
  inst_axis_interface->fifo_rdata(mop_rdata_signal);
  inst_axis_interface->axis_port.ConnectToPort(evrf_inst_interface);

  // Create two write-back slave AXI-streaming interfaces (receive write-back data from the loader module)
  wb_axis_interface = new axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<NARROW_WRITEBACK_BV_WIDTH>>(
      "evrf_wb_axis_interface", writeback_narrow, wb_interface_dataw, CORES, HIGH_PRECISION, DPES_PER_SECTOR);
  wb_axis_interface->clk(clk);
  wb_axis_interface->rst(rst);
  wb_axis_interface->fifo_rdy(wb_rdy_signal);
  wb_axis_interface->fifo_ren(wb_ren_signal);
  wb_axis_interface->fifo_rdata(wb_rdata_signal);
  wb_axis_interface->axis_port.ConnectToPort(evrf_wb_interface);

  // Create the input slave AXI-streaming interface (receives data from upstream module)
  ififo_axis_interface = new axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>(
      "evrf_ififo_interface", feedforward, ififo_interface_dataw, CORES, HIGH_PRECISION, DPES_PER_SECTOR);
  ififo_axis_interface->clk(clk);
  ififo_axis_interface->rst(rst);
  ififo_axis_interface->fifo_rdy(ififo_rdy_signal);
  ififo_axis_interface->fifo_ren(ififo_ren_signal);
  ififo_axis_interface->fifo_rdata(ififo_rdata_signal);
  ififo_axis_interface->axis_port.ConnectToPort(evrf_ififo_interface);

  // Create the output master AXI-streaming interface (send data to downstream module)
  ofifo_axis_interface = new axis_master_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>(
      "evrf_ofifo_interface", feedforward, ofifo_interface_dataw, CORES, HIGH_PRECISION, (sector_id * 6) + MFU0_DEST_ID,
      MFU_IN_STREAM_ID);
  ofifo_axis_interface->clk(clk);
  ofifo_axis_interface->rst(rst);
  ofifo_axis_interface->fifo_rdy(ofifo_rdy_signal);
  ofifo_axis_interface->fifo_ren(ofifo_ren_signal);
  ofifo_axis_interface->fifo_rdata(ofifo_rdata_signal);
  ofifo_axis_interface->axis_port.ConnectToPort(evrf_ofifo_interface);
}

axis_evrf::~axis_evrf() {
  delete evrf_module;
  delete inst_axis_interface;
  delete ififo_axis_interface;
  delete ofifo_axis_interface;
  delete wb_axis_interface;
}