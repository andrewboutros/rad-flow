#include <axis_loader.hpp>

axis_loader::axis_loader(const sc_module_name& name, unsigned int inst_interface_dataw,
                         unsigned int wb0_interface_dataw, unsigned int wb1_interface_dataw,
                         unsigned int ififo_interface_dataw, unsigned int id)
    : sc_module(name),
      ext_input_fifo_rdy("ext_input_fifo_rdy", CORES),
      ext_input_fifo_wen("ext_input_fifo_wen", CORES),
      ext_input_fifo_wdata("ext_input_fifo_wdata", CORES),
      ext_output_fifo_rdy("ext_output_fifo_rdy", CORES),
      ext_output_fifo_ren("ext_output_fifo_ren", CORES),
      ext_output_fifo_rdata("ext_output_fifo_rdata", CORES),
      input_fifo_rdy_signal("input_fifo_rdy_signal"),
      input_fifo_ren_signal("input_fifo_ren_signal"),
      input_fifo_rdata_signal("input_fifo_rdata_signal"),
      wb0_rdy_signal("wb0_rdy_signal", CORES),
      wb0_ren_signal("wb0_ren_signal", CORES),
      wb0_rdata_signal("wb0_rdata_signal", CORES),
      wb1_rdy_signal("wb1_rdy_signal"),
      wb1_ren_signal("wb1_ren_signal"),
      wb1_rdata_signal("wb1_rdata_signal"),
      uop_rdy_signal("uop_rdy_signal"),
      uop_wen_signal("uop_wen_signal"),
      uop_wdata_signal("uop_wdata_signal"),
      mop_rdy_signal("mop_rdy_signal", 1),
      mop_ren_signal("mop_ren_signal", 1),
      mop_rdata_signal("mop_rdata_signal", 1) {
  thread_id = id;
  
  init_vector<sc_signal<bool>>::init_sc_vector(input_fifo_rdy_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(input_fifo_ren_signal, SECTORS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(input_fifo_rdata_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(wb1_rdy_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(wb1_ren_signal, SECTORS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(wb1_rdata_signal, SECTORS, CORES);
  init_vector<axis_master_port>::init_sc_vector(loader_wb1_interfaces, SECTORS);
  init_vector<axis_slave_port>::init_sc_vector(loader_input_fifo_interfaces, SECTORS);

  // Create loader instruction decoder module
  decoder = new ld_decoder("ld_decoder");
  decoder->clk(clk);
  decoder->rst(rst);
  decoder->mop_rdy(mop_rdy_signal[0]);
  decoder->mop_ren(mop_ren_signal[0]);
  decoder->mop_rdata(mop_rdata_signal[0]);
  decoder->uop_rdy(uop_rdy_signal);
  decoder->uop_wen(uop_wen_signal);
  decoder->uop_wdata(uop_wdata_signal);

  // Create loader module
  loader_module = new loader("ld", thread_id);
  loader_module->clk(clk);
  loader_module->rst(rst);
  loader_module->inst(uop_wdata_signal);
  loader_module->inst_fifo_wen(uop_wen_signal);
  loader_module->inst_fifo_rdy(uop_rdy_signal);
  loader_module->wb0_fifo_rdy(wb0_rdy_signal);
  loader_module->wb0_fifo_ren(wb0_ren_signal);
  loader_module->wb0_fifo_rdata(wb0_rdata_signal);
  loader_module->wb1_fifo_rdy(wb1_rdy_signal);
  loader_module->wb1_fifo_ren(wb1_ren_signal);
  loader_module->wb1_fifo_rdata(wb1_rdata_signal);
  loader_module->input_fifo_rdy(input_fifo_rdy_signal);
  loader_module->input_fifo_ren(input_fifo_ren_signal);
  loader_module->input_fifo_rdata(input_fifo_rdata_signal);
  loader_module->ext_input_fifo_rdy(ext_input_fifo_rdy);
  loader_module->ext_input_fifo_wen(ext_input_fifo_wen);
  loader_module->ext_input_fifo_wdata(ext_input_fifo_wdata);
  loader_module->ext_output_fifo_rdy(ext_output_fifo_rdy);
  loader_module->ext_output_fifo_ren(ext_output_fifo_ren);
  loader_module->ext_output_fifo_rdata(ext_output_fifo_rdata);

  // Create the instruction slave AXI-streaming interface (receives macro-ops from top-level instruction dispatcher)
  inst_axis_interface = new axis_slave_fifo_adapter<ld_mop, sc_bv<LD_MOP_BITWIDTH>>(
      "ld_inst_axis_interface", inst, inst_interface_dataw, 1, LD_MOP_BITWIDTH, 1);
  inst_axis_interface->clk(clk);
  inst_axis_interface->rst(rst);
  inst_axis_interface->fifo_rdy(mop_rdy_signal);
  inst_axis_interface->fifo_ren(mop_ren_signal);
  inst_axis_interface->fifo_rdata(mop_rdata_signal);
  inst_axis_interface->axis_port.ConnectToPort(loader_inst_interface);

  // Create two write-back master AXI-streaming interfaces (send write-back data to different NPU modules)
  wb0_axis_interface = new axis_master_fifo_adapter<data_vector<tb_input_precision>, sc_bv<WIDE_WRITEBACK_BV_WIDTH>>(
      "ld_wb0_axis_interface", writeback_wide, wb0_interface_dataw, CORES, LOW_PRECISION, SECTOR_DEST_ID,
      SECTOR_WB_STREAM_ID + thread_id);
  wb0_axis_interface->clk(clk);
  wb0_axis_interface->rst(rst);
  wb0_axis_interface->fifo_rdy(wb0_rdy_signal);
  wb0_axis_interface->fifo_ren(wb0_ren_signal);
  wb0_axis_interface->fifo_rdata(wb0_rdata_signal);
  wb0_axis_interface->axis_port.ConnectToPort(loader_wb0_interface);

  char module_name[NAME_LENGTH];
  std::string module_name_str;
  wb1_axis_interfaces.resize(SECTORS);
  input_fifo_axis_interfaces.resize(SECTORS);
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    module_name_str = "ld_wb1_axis_interface_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    wb1_axis_interfaces[sector_id] =
        new axis_master_fifo_adapter<data_vector<tb_output_precision>, sc_bv<NARROW_WRITEBACK_BV_WIDTH>>(
            module_name, writeback_narrow, wb1_interface_dataw, CORES, HIGH_PRECISION, 0, EVRF_WB_STREAM_ID);
    wb1_axis_interfaces[sector_id]->clk(clk);
    wb1_axis_interfaces[sector_id]->rst(rst);
    wb1_axis_interfaces[sector_id]->fifo_rdy(wb1_rdy_signal[sector_id]);
    wb1_axis_interfaces[sector_id]->fifo_ren(wb1_ren_signal[sector_id]);
    wb1_axis_interfaces[sector_id]->fifo_rdata(wb1_rdata_signal[sector_id]);
    wb1_axis_interfaces[sector_id]->axis_port.ConnectToPort(loader_wb1_interfaces[sector_id]);

    module_name_str = "ld_input_fifo_interface_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    input_fifo_axis_interfaces[sector_id] =
        new axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>(
            module_name, feedforward, ififo_interface_dataw, CORES, HIGH_PRECISION, DPES_PER_SECTOR);
    input_fifo_axis_interfaces[sector_id]->clk(clk);
    input_fifo_axis_interfaces[sector_id]->rst(rst);
    input_fifo_axis_interfaces[sector_id]->fifo_rdy(input_fifo_rdy_signal[sector_id]);
    input_fifo_axis_interfaces[sector_id]->fifo_ren(input_fifo_ren_signal[sector_id]);
    input_fifo_axis_interfaces[sector_id]->fifo_rdata(input_fifo_rdata_signal[sector_id]);
    input_fifo_axis_interfaces[sector_id]->axis_port.ConnectToPort(loader_input_fifo_interfaces[sector_id]);
  }
}

axis_loader::~axis_loader() {
  delete loader_module;
  delete inst_axis_interface;
  delete wb0_axis_interface;
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    delete input_fifo_axis_interfaces[sector_id];
    delete wb1_axis_interfaces[sector_id];
  }
}