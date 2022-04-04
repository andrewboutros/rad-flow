#include <axis_mfu.hpp>

axis_mfu::axis_mfu(const sc_module_name& name, unsigned int id_mfu, unsigned int id_sector, unsigned int id_thread,
                   unsigned int inst_interface_dataw, unsigned int wb_interface_dataw,
                   unsigned int ififo_interface_dataw, unsigned int ofifo_interface_dataw)
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
  mfu_id = id_mfu;
  sector_id = id_sector;
  thread_id = id_thread;

  // Create MFU instruction decoder module
  decoder = new mfu_decoder("mfu_decoder");
  decoder->clk(clk);
  decoder->rst(rst);
  decoder->mop_rdy(mop_rdy_signal[0]);
  decoder->mop_ren(mop_ren_signal[0]);
  decoder->mop_rdata(mop_rdata_signal[0]);
  decoder->uop_rdy(uop_rdy_signal);
  decoder->uop_wen(uop_wen_signal);
  decoder->uop_wdata(uop_wdata_signal);

  // Create MFU module
  mfu_module = new mfu("mfu_module", mfu_id, sector_id, thread_id);
  mfu_module->clk(clk);
  mfu_module->rst(rst);
  mfu_module->inst(uop_wdata_signal);
  mfu_module->inst_fifo_wen(uop_wen_signal);
  mfu_module->inst_fifo_rdy(uop_rdy_signal);
  mfu_module->wbfifo_rdy(wb_rdy_signal);
  mfu_module->wbfifo_ren(wb_ren_signal);
  mfu_module->wbfifo_rdata(wb_rdata_signal);
  mfu_module->input_ofifo_rdy(ififo_rdy_signal);
  mfu_module->input_ofifo_ren(ififo_ren_signal);
  mfu_module->input_ofifo_rdata(ififo_rdata_signal);
  mfu_module->mfu_ofifo_rdy(ofifo_rdy_signal);
  mfu_module->mfu_ofifo_ren(ofifo_ren_signal);
  mfu_module->mfu_result(ofifo_rdata_signal);

  // Create the instruction slave AXI-streaming interface (receives macro-ops from top-level instruction dispatcher)
  inst_axis_interface = new axis_slave_fifo_adapter<mfu_mop, sc_bv<MFU_MOP_BITWIDTH>>(
      "mfu_inst_interface", inst, inst_interface_dataw, 1, MFU_MOP_BITWIDTH, 1);
  inst_axis_interface->clk(clk);
  inst_axis_interface->rst(rst);
  inst_axis_interface->fifo_rdy(mop_rdy_signal);
  inst_axis_interface->fifo_ren(mop_ren_signal);
  inst_axis_interface->fifo_rdata(mop_rdata_signal);
  inst_axis_interface->axis_port.ConnectToPort(mfu_inst_interface);

  // Create two write-back slave AXI-streaming interfaces (receive write-back data from the loader module)
  wb_axis_interface = new axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<NARROW_WRITEBACK_BV_WIDTH>>(
      "mfu_wb_interface", writeback_narrow, wb_interface_dataw, CORES, HIGH_PRECISION, DPES_PER_SECTOR);
  wb_axis_interface->clk(clk);
  wb_axis_interface->rst(rst);
  wb_axis_interface->fifo_rdy(wb_rdy_signal);
  wb_axis_interface->fifo_ren(wb_ren_signal);
  wb_axis_interface->fifo_rdata(wb_rdata_signal);
  wb_axis_interface->axis_port.ConnectToPort(mfu_wb_interface);

  // Create the input slave AXI-streaming interface (receives data from upstream module)
  ififo_axis_interface = new axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>(
      "mfu_ififo_interface", feedforward, ififo_interface_dataw, CORES, HIGH_PRECISION, DPES_PER_SECTOR);
  ififo_axis_interface->clk(clk);
  ififo_axis_interface->rst(rst);
  ififo_axis_interface->fifo_rdy(ififo_rdy_signal);
  ififo_axis_interface->fifo_ren(ififo_ren_signal);
  ififo_axis_interface->fifo_rdata(ififo_rdata_signal);
  ififo_axis_interface->axis_port.ConnectToPort(mfu_ififo_interface);

  // Create the output master AXI-streaming interface (send data to downstream module)
  unsigned int destination_id = (mfu_id == 0) ? (sector_id * 6) + MFU1_DEST_ID : (sector_id * 6) + LD_DEST_ID;
  unsigned int stream_id = (mfu_id == 0) ? MFU_IN_STREAM_ID : LD_IN_STREAM_ID;
  ofifo_axis_interface = new axis_master_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>(
      "mfu_ofifo_interface", feedforward, ofifo_interface_dataw, CORES, HIGH_PRECISION, destination_id, stream_id);
  ofifo_axis_interface->clk(clk);
  ofifo_axis_interface->rst(rst);
  ofifo_axis_interface->fifo_rdy(ofifo_rdy_signal);
  ofifo_axis_interface->fifo_ren(ofifo_ren_signal);
  ofifo_axis_interface->fifo_rdata(ofifo_rdata_signal);
  ofifo_axis_interface->axis_port.ConnectToPort(mfu_ofifo_interface);
}

axis_mfu::~axis_mfu() {
  delete mfu_module;
  delete inst_axis_interface;
  delete ififo_axis_interface;
  delete ofifo_axis_interface;
  delete wb_axis_interface;
}