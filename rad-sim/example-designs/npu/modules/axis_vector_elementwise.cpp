#include <axis_vector_elementwise.hpp>

axis_vector_elementwise::axis_vector_elementwise(const sc_module_name& name, unsigned int thread_id)
    : radsim_module(name),
      evrf_ififo_rdy_signal("evrf_ififo_rdy_signal"),
      evrf_ififo_ren_signal("evrf_ififo_ren_signal"),
      evrf_ififo_rdata_signal("evrf_ififo_rdata_signal"),
      mfu0_ififo_rdy_signal("mfu0_ififo_rdy_signal"),
      mfu0_ififo_ren_signal("mfu0_ififo_ren_signal"),
      mfu0_ififo_rdata_signal("mfu0_ififo_rdata_signal"),
      mfu1_ififo_rdy_signal("mfu1_ififo_rdy_signal"),
      mfu1_ififo_ren_signal("mfu1_ififo_ren_signal"),
      mfu1_ififo_rdata_signal("mfu1_ififo_rdata_signal"),
      ld_ififo_rdy_signal("ld_ififo_rdy_signal"),
      ld_ififo_ren_signal("ld_ififo_ren_signal"),
      ld_ififo_rdata_signal("ld_ififo_rdata_signal"),
      ld_wb0_rdy_signal("ld_wb0_rdy_signal", CORES),
      ld_wb0_ren_signal("ld_wb0_ren_signal", CORES),
      ld_wb0_rdata_signal("ld_wb0_rdata_signal", CORES),
      ld_wb1_rdy_signal("ld_wb1_rdy_signal"),
      ld_wb1_ren_signal("ld_wb1_ren_signal"),
      ld_wb1_rdata_signal("ld_wb1_rdata_signal"),
      evrf_wb1_ren_signal("evrf_wb1_ren_signal"),
      mfu0_wb1_ren_signal("mfu0_wb1_ren_signal"),
      mfu1_wb1_ren_signal("mfu1_wb1_ren_signal"),
      evrf_mop_rdy_signal("evrf_mop_rdy_signal"),
      evrf_mop_ren_signal("evrf_mop_ren_signal"),
      evrf_mop_rdata_signal("evrf_mop_rdata_signal"),
      evrf_uop_rdy_signal("evrf_uop_rdy_signal", SECTORS),
      evrf_uop_wen_signal("evrf_uop_wen_signal", SECTORS),
      evrf_uop_wdata_signal("evrf_uop_wdata_signal", SECTORS),
      mfu0_mop_rdy_signal("mfu0_mop_rdy_signal"),
      mfu0_mop_ren_signal("mfu0_mop_ren_signal"),
      mfu0_mop_rdata_signal("mfu0_mop_rdata_signal"),
      mfu0_uop_rdy_signal("mfu0_uop_rdy_signal", SECTORS),
      mfu0_uop_wen_signal("mfu0_uop_wen_signal", SECTORS),
      mfu0_uop_wdata_signal("mfu0_uop_wdata_signal", SECTORS),
      mfu1_mop_rdy_signal("mfu1_mop_rdy_signal"),
      mfu1_mop_ren_signal("mfu1_mop_ren_signal"),
      mfu1_mop_rdata_signal("mfu1_mop_rdata_signal"),
      mfu1_uop_rdy_signal("mfu1_uop_rdy_signal", SECTORS),
      mfu1_uop_wen_signal("mfu1_uop_wen_signal", SECTORS),
      mfu1_uop_wdata_signal("mfu1_uop_wdata_signal", SECTORS),
      ld_mop_rdy_signal("ld_mop_rdy_signal", 1),
      ld_mop_ren_signal("ld_mop_ren_signal", 1),
      ld_mop_rdata_signal("ld_mop_rdata_signal", 1),
      ld_uop_rdy_signal("ld_uop_rdy_signal"),
      ld_uop_wen_signal("ld_uop_wen_signal"),
      ld_uop_wdata_signal("ld_uop_wdata_signal"),
      ext_input_fifo_rdy("ext_input_fifo_rdy", CORES),
      ext_input_fifo_wen("ext_input_fifo_wen", CORES),
      ext_input_fifo_wdata("ext_input_fifo_wdata", CORES),
      ext_output_fifo_rdy("ext_output_fifo_rdy", CORES),
      ext_output_fifo_ren("ext_output_fifo_ren", CORES),
      ext_output_fifo_rdata("ext_output_fifo_rdata", CORES) {

  _name = name;
  _thread_id = thread_id;

  init_vector<axis_slave_port>::init_sc_vector(evrf_inst_interfaces, SECTORS);
  init_vector<axis_slave_port>::init_sc_vector(mfu0_inst_interfaces, SECTORS);
  init_vector<axis_slave_port>::init_sc_vector(mfu1_inst_interfaces, SECTORS);
  init_vector<axis_slave_port>::init_sc_vector(evrf_ififo_interfaces, SECTORS);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_ififo_rdy_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_ififo_ren_signal, SECTORS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(evrf_ififo_rdata_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_ififo_rdy_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_ififo_ren_signal, SECTORS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(mfu0_ififo_rdata_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_ififo_rdy_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_ififo_ren_signal, SECTORS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(mfu1_ififo_rdata_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ld_ififo_rdy_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ld_ififo_ren_signal, SECTORS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(ld_ififo_rdata_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ld_wb1_rdy_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ld_wb1_ren_signal, SECTORS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(ld_wb1_rdata_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_mop_rdy_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_mop_ren_signal, SECTORS, 1);
  init_vector<sc_signal<evrf_mop>>::init_sc_vector(evrf_mop_rdata_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_mop_rdy_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_mop_ren_signal, SECTORS, 1);
  init_vector<sc_signal<mfu_mop>>::init_sc_vector(mfu0_mop_rdata_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_mop_rdy_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_mop_ren_signal, SECTORS, 1);
  init_vector<sc_signal<mfu_mop>>::init_sc_vector(mfu1_mop_rdata_signal, SECTORS, 1);
  init_vector<sc_signal<bool>>::init_sc_vector(evrf_wb1_ren_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu0_wb1_ren_signal, SECTORS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(mfu1_wb1_ren_signal, SECTORS, CORES);

  evrf_inst_decoders.resize(SECTORS);
  mfu0_inst_decoders.resize(SECTORS);
  mfu1_inst_decoders.resize(SECTORS);
  evrf_modules.resize(SECTORS);
  mfu0_modules.resize(SECTORS);
  mfu1_modules.resize(SECTORS);
  evrf_inst_axis_interfaces.resize(SECTORS);
  mfu0_inst_axis_interfaces.resize(SECTORS);
  mfu1_inst_axis_interfaces.resize(SECTORS);
  evrf_ififo_axis_interfaces.resize(SECTORS);
  char module_name[NAME_LENGTH];
  std::string module_name_str;

  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    module_name_str = "evrf_inst_decoder_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    evrf_inst_decoders[sector_id] = new evrf_decoder(module_name);
    evrf_inst_decoders[sector_id]->clk(clk);
    evrf_inst_decoders[sector_id]->rst(rst);
    evrf_inst_decoders[sector_id]->mop_rdy(evrf_mop_rdy_signal[sector_id][0]);
    evrf_inst_decoders[sector_id]->mop_ren(evrf_mop_ren_signal[sector_id][0]);
    evrf_inst_decoders[sector_id]->mop_rdata(evrf_mop_rdata_signal[sector_id][0]);
    evrf_inst_decoders[sector_id]->uop_rdy(evrf_uop_rdy_signal[sector_id]);
    evrf_inst_decoders[sector_id]->uop_wen(evrf_uop_wen_signal[sector_id]);
    evrf_inst_decoders[sector_id]->uop_wdata(evrf_uop_wdata_signal[sector_id]);

    module_name_str = "evrf_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    evrf_modules[sector_id] = new evrf(module_name, sector_id, _thread_id);
    evrf_modules[sector_id]->clk(clk);
    evrf_modules[sector_id]->rst(rst);
    evrf_modules[sector_id]->inst(evrf_uop_wdata_signal[sector_id]);
    evrf_modules[sector_id]->inst_fifo_wen(evrf_uop_wen_signal[sector_id]);
    evrf_modules[sector_id]->inst_fifo_rdy(evrf_uop_rdy_signal[sector_id]);
    evrf_modules[sector_id]->wbfifo_rdy(ld_wb1_rdy_signal[sector_id]);
    evrf_modules[sector_id]->wbfifo_ren(evrf_wb1_ren_signal[sector_id]);
    evrf_modules[sector_id]->wbfifo_rdata(ld_wb1_rdata_signal[sector_id]);
    evrf_modules[sector_id]->mvu_ofifo_rdy(evrf_ififo_rdy_signal[sector_id]);
    evrf_modules[sector_id]->mvu_ofifo_ren(evrf_ififo_ren_signal[sector_id]);
    evrf_modules[sector_id]->mvu_ofifo_rdata(evrf_ififo_rdata_signal[sector_id]);
    evrf_modules[sector_id]->evrf_ofifo_rdy(mfu0_ififo_rdy_signal[sector_id]);
    evrf_modules[sector_id]->evrf_ofifo_ren(mfu0_ififo_ren_signal[sector_id]);
    evrf_modules[sector_id]->evrf_result(mfu0_ififo_rdata_signal[sector_id]);

    module_name_str = "evrf_inst_axis_interface_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    evrf_inst_axis_interfaces[sector_id] = new axis_slave_fifo_adapter<evrf_mop, sc_bv<EVRF_MOP_BITWIDTH>>(
        module_name, INSTRUCTION_INTERFACE, VEW_INSTRUCTION_INTERFACE_DATAW, 1, EVRF_MOP_BITWIDTH, 1);
    evrf_inst_axis_interfaces[sector_id]->clk(clk);
    evrf_inst_axis_interfaces[sector_id]->rst(rst);
    evrf_inst_axis_interfaces[sector_id]->fifo_rdy(evrf_mop_rdy_signal[sector_id]);
    evrf_inst_axis_interfaces[sector_id]->fifo_ren(evrf_mop_ren_signal[sector_id]);
    evrf_inst_axis_interfaces[sector_id]->fifo_rdata(evrf_mop_rdata_signal[sector_id]);
    evrf_inst_axis_interfaces[sector_id]->axis_port.ConnectToPort(evrf_inst_interfaces[sector_id]);

    module_name_str = "evrf_ififo_interface_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    evrf_ififo_axis_interfaces[sector_id] =
        new axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>(
            module_name, FEEDFORWARD_INTERFACE, VEW_FEEDFORWARD_INTERFACE_DATAW, CORES, HIGH_PRECISION, DPES_PER_SECTOR);
    evrf_ififo_axis_interfaces[sector_id]->clk(clk);
    evrf_ififo_axis_interfaces[sector_id]->rst(rst);
    evrf_ififo_axis_interfaces[sector_id]->fifo_rdy(evrf_ififo_rdy_signal[sector_id]);
    evrf_ififo_axis_interfaces[sector_id]->fifo_ren(evrf_ififo_ren_signal[sector_id]);
    evrf_ififo_axis_interfaces[sector_id]->fifo_rdata(evrf_ififo_rdata_signal[sector_id]);
    evrf_ififo_axis_interfaces[sector_id]->axis_port.ConnectToPort(evrf_ififo_interfaces[sector_id]);

    module_name_str = "mfu0_decoder_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    mfu0_inst_decoders[sector_id] = new mfu_decoder(module_name);
    mfu0_inst_decoders[sector_id]->clk(clk);
    mfu0_inst_decoders[sector_id]->rst(rst);
    mfu0_inst_decoders[sector_id]->mop_rdy(mfu0_mop_rdy_signal[sector_id][0]);
    mfu0_inst_decoders[sector_id]->mop_ren(mfu0_mop_ren_signal[sector_id][0]);
    mfu0_inst_decoders[sector_id]->mop_rdata(mfu0_mop_rdata_signal[sector_id][0]);
    mfu0_inst_decoders[sector_id]->uop_rdy(mfu0_uop_rdy_signal[sector_id]);
    mfu0_inst_decoders[sector_id]->uop_wen(mfu0_uop_wen_signal[sector_id]);
    mfu0_inst_decoders[sector_id]->uop_wdata(mfu0_uop_wdata_signal[sector_id]);

    module_name_str = "mfu0_module_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    mfu0_modules[sector_id] = new mfu(module_name, 0, sector_id, _thread_id);
    mfu0_modules[sector_id]->clk(clk);
    mfu0_modules[sector_id]->rst(rst);
    mfu0_modules[sector_id]->inst(mfu0_uop_wdata_signal[sector_id]);
    mfu0_modules[sector_id]->inst_fifo_wen(mfu0_uop_wen_signal[sector_id]);
    mfu0_modules[sector_id]->inst_fifo_rdy(mfu0_uop_rdy_signal[sector_id]);
    mfu0_modules[sector_id]->wbfifo_rdy(ld_wb1_rdy_signal[sector_id]);
    mfu0_modules[sector_id]->wbfifo_ren(mfu0_wb1_ren_signal[sector_id]);
    mfu0_modules[sector_id]->wbfifo_rdata(ld_wb1_rdata_signal[sector_id]);
    mfu0_modules[sector_id]->input_ofifo_rdy(mfu0_ififo_rdy_signal[sector_id]);
    mfu0_modules[sector_id]->input_ofifo_ren(mfu0_ififo_ren_signal[sector_id]);
    mfu0_modules[sector_id]->input_ofifo_rdata(mfu0_ififo_rdata_signal[sector_id]);
    mfu0_modules[sector_id]->mfu_ofifo_rdy(mfu1_ififo_rdy_signal[sector_id]);
    mfu0_modules[sector_id]->mfu_ofifo_ren(mfu1_ififo_ren_signal[sector_id]);
    mfu0_modules[sector_id]->mfu_result(mfu1_ififo_rdata_signal[sector_id]);

    module_name_str = "mfu0_inst_interface_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    mfu0_inst_axis_interfaces[sector_id] = new axis_slave_fifo_adapter<mfu_mop, sc_bv<MFU_MOP_BITWIDTH>>(
        module_name, INSTRUCTION_INTERFACE, VEW_INSTRUCTION_INTERFACE_DATAW, 1, MFU_MOP_BITWIDTH, 1);
    mfu0_inst_axis_interfaces[sector_id]->clk(clk);
    mfu0_inst_axis_interfaces[sector_id]->rst(rst);
    mfu0_inst_axis_interfaces[sector_id]->fifo_rdy(mfu0_mop_rdy_signal[sector_id]);
    mfu0_inst_axis_interfaces[sector_id]->fifo_ren(mfu0_mop_ren_signal[sector_id]);
    mfu0_inst_axis_interfaces[sector_id]->fifo_rdata(mfu0_mop_rdata_signal[sector_id]);
    mfu0_inst_axis_interfaces[sector_id]->axis_port.ConnectToPort(mfu0_inst_interfaces[sector_id]);

    module_name_str = "mfu1_decoder_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    mfu1_inst_decoders[sector_id] = new mfu_decoder(module_name);
    mfu1_inst_decoders[sector_id]->clk(clk);
    mfu1_inst_decoders[sector_id]->rst(rst);
    mfu1_inst_decoders[sector_id]->mop_rdy(mfu1_mop_rdy_signal[sector_id][0]);
    mfu1_inst_decoders[sector_id]->mop_ren(mfu1_mop_ren_signal[sector_id][0]);
    mfu1_inst_decoders[sector_id]->mop_rdata(mfu1_mop_rdata_signal[sector_id][0]);
    mfu1_inst_decoders[sector_id]->uop_rdy(mfu1_uop_rdy_signal[sector_id]);
    mfu1_inst_decoders[sector_id]->uop_wen(mfu1_uop_wen_signal[sector_id]);
    mfu1_inst_decoders[sector_id]->uop_wdata(mfu1_uop_wdata_signal[sector_id]);

    module_name_str = "mfu1_module_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    mfu1_modules[sector_id] = new mfu(module_name, 1, sector_id, _thread_id);
    mfu1_modules[sector_id]->clk(clk);
    mfu1_modules[sector_id]->rst(rst);
    mfu1_modules[sector_id]->inst(mfu1_uop_wdata_signal[sector_id]);
    mfu1_modules[sector_id]->inst_fifo_wen(mfu1_uop_wen_signal[sector_id]);
    mfu1_modules[sector_id]->inst_fifo_rdy(mfu1_uop_rdy_signal[sector_id]);
    mfu1_modules[sector_id]->wbfifo_rdy(ld_wb1_rdy_signal[sector_id]);
    mfu1_modules[sector_id]->wbfifo_ren(mfu1_wb1_ren_signal[sector_id]);
    mfu1_modules[sector_id]->wbfifo_rdata(ld_wb1_rdata_signal[sector_id]);
    mfu1_modules[sector_id]->input_ofifo_rdy(mfu1_ififo_rdy_signal[sector_id]);
    mfu1_modules[sector_id]->input_ofifo_ren(mfu1_ififo_ren_signal[sector_id]);
    mfu1_modules[sector_id]->input_ofifo_rdata(mfu1_ififo_rdata_signal[sector_id]);
    mfu1_modules[sector_id]->mfu_ofifo_rdy(ld_ififo_rdy_signal[sector_id]);
    mfu1_modules[sector_id]->mfu_ofifo_ren(ld_ififo_ren_signal[sector_id]);
    mfu1_modules[sector_id]->mfu_result(ld_ififo_rdata_signal[sector_id]);

    module_name_str = "mfu1_inst_interface_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    mfu1_inst_axis_interfaces[sector_id] = new axis_slave_fifo_adapter<mfu_mop, sc_bv<MFU_MOP_BITWIDTH>>(
        module_name, INSTRUCTION_INTERFACE, VEW_INSTRUCTION_INTERFACE_DATAW, 1, MFU_MOP_BITWIDTH, 1);
    mfu1_inst_axis_interfaces[sector_id]->clk(clk);
    mfu1_inst_axis_interfaces[sector_id]->rst(rst);
    mfu1_inst_axis_interfaces[sector_id]->fifo_rdy(mfu1_mop_rdy_signal[sector_id]);
    mfu1_inst_axis_interfaces[sector_id]->fifo_ren(mfu1_mop_ren_signal[sector_id]);
    mfu1_inst_axis_interfaces[sector_id]->fifo_rdata(mfu1_mop_rdata_signal[sector_id]);
    mfu1_inst_axis_interfaces[sector_id]->axis_port.ConnectToPort(mfu1_inst_interfaces[sector_id]);
  }

  ld_inst_decoder = new ld_decoder("ld_decoder");
  ld_inst_decoder->clk(clk);
  ld_inst_decoder->rst(rst);
  ld_inst_decoder->mop_rdy(ld_mop_rdy_signal[0]);
  ld_inst_decoder->mop_ren(ld_mop_ren_signal[0]);
  ld_inst_decoder->mop_rdata(ld_mop_rdata_signal[0]);
  ld_inst_decoder->uop_rdy(ld_uop_rdy_signal);
  ld_inst_decoder->uop_wen(ld_uop_wen_signal);
  ld_inst_decoder->uop_wdata(ld_uop_wdata_signal);

  ld_module = new loader("ld");
  ld_module->clk(clk);
  ld_module->rst(rst);
  ld_module->inst(ld_uop_wdata_signal);
  ld_module->inst_fifo_wen(ld_uop_wen_signal);
  ld_module->inst_fifo_rdy(ld_uop_rdy_signal);
  ld_module->wb0_fifo_rdy(ld_wb0_rdy_signal);
  ld_module->wb0_fifo_ren(ld_wb0_ren_signal);
  ld_module->wb0_fifo_rdata(ld_wb0_rdata_signal);
  ld_module->wb1_fifo_rdy(ld_wb1_rdy_signal);
  ld_module->wb1_fifo_ren(ld_wb1_ren_signal);
  ld_module->wb1_fifo_rdata(ld_wb1_rdata_signal);
  ld_module->input_fifo_rdy(ld_ififo_rdy_signal);
  ld_module->input_fifo_ren(ld_ififo_ren_signal);
  ld_module->input_fifo_rdata(ld_ififo_rdata_signal);
  ld_module->ext_input_fifo_rdy(ext_input_fifo_rdy);
  ld_module->ext_input_fifo_wen(ext_input_fifo_wen);
  ld_module->ext_input_fifo_wdata(ext_input_fifo_wdata);
  ld_module->ext_output_fifo_rdy(ext_output_fifo_rdy);
  ld_module->ext_output_fifo_ren(ext_output_fifo_ren);
  ld_module->ext_output_fifo_rdata(ext_output_fifo_rdata);

  ld_inst_axis_interface = new axis_slave_fifo_adapter<ld_mop, sc_bv<LD_MOP_BITWIDTH>>(
      "ld_inst_axis_interface", INSTRUCTION_INTERFACE, VEW_INSTRUCTION_INTERFACE_DATAW, 1, LD_MOP_BITWIDTH, 1);
  ld_inst_axis_interface->clk(clk);
  ld_inst_axis_interface->rst(rst);
  ld_inst_axis_interface->fifo_rdy(ld_mop_rdy_signal);
  ld_inst_axis_interface->fifo_ren(ld_mop_ren_signal);
  ld_inst_axis_interface->fifo_rdata(ld_mop_rdata_signal);
  ld_inst_axis_interface->axis_port.ConnectToPort(loader_inst_interface);

  // Create two write-back master AXI-streaming interfaces (send write-back data to different NPU modules)
  std::string dest_name = "axis_mvu_sector_0.sector_wb_interface_" + std::to_string(_thread_id);
  ld_wb0_axis_interface = new axis_master_fifo_adapter<data_vector<tb_input_precision>, sc_bv<WIDE_WRITEBACK_BV_WIDTH>>(
      "ld_wb0_axis_interface", MVU_WRITEBACK_INTERFACE, VEW_WB0_INTERFACE_DATAW, CORES, LOW_PRECISION, dest_name);
  ld_wb0_axis_interface->clk(clk);
  ld_wb0_axis_interface->rst(rst);
  ld_wb0_axis_interface->fifo_rdy(ld_wb0_rdy_signal);
  ld_wb0_axis_interface->fifo_ren(ld_wb0_ren_signal);
  ld_wb0_axis_interface->fifo_rdata(ld_wb0_rdata_signal);
  ld_wb0_axis_interface->axis_port.ConnectToPort(loader_wb0_interface);

  SC_METHOD(Assign);
  sensitive << rst;
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    for (unsigned int core_id = 0; core_id < CORES; core_id++) {
      sensitive << evrf_wb1_ren_signal[sector_id][core_id] << mfu0_wb1_ren_signal[sector_id][core_id]
                << mfu1_wb1_ren_signal[sector_id][core_id];
    }
  }

  this->RegisterModuleInfo();
}

axis_vector_elementwise::~axis_vector_elementwise() {
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    delete evrf_inst_decoders[sector_id];
    delete mfu0_inst_decoders[sector_id];
    delete mfu1_inst_decoders[sector_id];
    delete evrf_modules[sector_id];
    delete mfu0_modules[sector_id];
    delete mfu1_modules[sector_id];
    delete evrf_inst_axis_interfaces[sector_id];
    delete mfu0_inst_axis_interfaces[sector_id];
    delete mfu1_inst_axis_interfaces[sector_id];
    delete evrf_ififo_axis_interfaces[sector_id];
  }
  delete ld_inst_decoder;
  delete ld_module;
  delete ld_inst_axis_interface;
  delete ld_wb0_axis_interface;
}

void axis_vector_elementwise::Assign() {
  if (rst.read()) {
    for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
      for (unsigned int core_id = 0; core_id < CORES; core_id++) {
        ld_wb1_ren_signal[sector_id][core_id].write(false);
      }
    }
  } else {
    for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
      for (unsigned int core_id = 0; core_id < CORES; core_id++) {
        bool ren_value = evrf_wb1_ren_signal[sector_id][core_id].read() ||
                         mfu0_wb1_ren_signal[sector_id][core_id].read() ||
                         mfu1_wb1_ren_signal[sector_id][core_id].read();
        ld_wb1_ren_signal[sector_id][core_id].write(ren_value);
      }
    }
  }
}

void axis_vector_elementwise::RegisterModuleInfo() {
  std::string port_name;
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    port_name = _name + ".evrf_inst_interface_" + std::to_string(sector_id);
    RegisterSlavePort(port_name, &evrf_inst_interfaces[sector_id], VEW_INSTRUCTION_INTERFACE_DATAW, INSTRUCTION_INTERFACE);

    port_name = _name + ".mfu0_inst_interface_" + std::to_string(sector_id);
    RegisterSlavePort(port_name, &mfu0_inst_interfaces[sector_id], VEW_INSTRUCTION_INTERFACE_DATAW, INSTRUCTION_INTERFACE);

    port_name = _name + ".mfu1_inst_interface_" + std::to_string(sector_id);
    RegisterSlavePort(port_name, &mfu1_inst_interfaces[sector_id], VEW_INSTRUCTION_INTERFACE_DATAW, INSTRUCTION_INTERFACE);
  }
  port_name = _name + ".loader_inst_interface";
  RegisterSlavePort(port_name, &loader_inst_interface, VEW_INSTRUCTION_INTERFACE_DATAW, INSTRUCTION_INTERFACE);

  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    port_name = _name + ".evrf_ififo_interface_" + std::to_string(sector_id);
    RegisterSlavePort(port_name, &evrf_ififo_interfaces[sector_id], VEW_FEEDFORWARD_INTERFACE_DATAW, FEEDFORWARD_INTERFACE);
  }

  port_name = _name + ".loader_wb0_interface";
  RegisterMasterPort(port_name, &loader_wb0_interface, VEW_WB0_INTERFACE_DATAW, MVU_WRITEBACK_INTERFACE);
}