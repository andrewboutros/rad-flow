#include <design.hpp>

#define INST_AXIS_DATAW 256
#define WB0_AXIS_DATAW 256
#define WB1_AXIS_DATAW 256
#define BLOCKS_FF_AXIS_DATAW 256

#define SECTOR_INST_AXIS_DATAW 256
#define SECTOR_FF_AXIS_DATAW 256
#define SECTOR_WB0_AXIS_DATAW 256

void design::prepare_adapters_info() {
  std::vector<Flit::FlitType> first_sector_input_interface_types, first_sector_output_interface_types;
  std::vector<unsigned int> first_sector_input_interface_dataw, first_sector_output_interface_dataw;

  std::vector<Flit::FlitType> sector_input_interface_types, sector_output_interface_types;
  std::vector<unsigned int> sector_input_interface_dataw, sector_output_interface_dataw;

  std::vector<Flit::FlitType> vector_elementwise_input_interface_types;
  std::vector<unsigned int> vector_elementwise_input_interface_dataw;

  std::vector<Flit::FlitType> ld_input_interface_types, ld_output_interface_types;
  std::vector<unsigned int> ld_input_interface_dataw, ld_output_interface_dataw;

  std::vector<Flit::FlitType> inst_dispatcher_output_interface_types;
  std::vector<unsigned int> inst_dispatcher_output_interface_dataw;

  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    first_sector_input_interface_types.push_back(AXIS_TYPE0);
    first_sector_input_interface_dataw.push_back(SECTOR_INST_AXIS_DATAW);
  }
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    first_sector_input_interface_types.push_back(AXIS_TYPE1);
    first_sector_input_interface_dataw.push_back(SECTOR_WB0_AXIS_DATAW);
  }
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    first_sector_output_interface_types.push_back(AXIS_TYPE2);
    first_sector_output_interface_dataw.push_back(SECTOR_FF_AXIS_DATAW);
  }

  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    sector_input_interface_types.push_back(AXIS_TYPE0);
    sector_input_interface_dataw.push_back(SECTOR_INST_AXIS_DATAW);
  }
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    sector_output_interface_types.push_back(AXIS_TYPE2);
    sector_output_interface_dataw.push_back(SECTOR_FF_AXIS_DATAW);
  }

  vector_elementwise_input_interface_types.push_back(AXIS_TYPE0);
  vector_elementwise_input_interface_dataw.push_back(INST_AXIS_DATAW);
  vector_elementwise_input_interface_types.push_back(AXIS_TYPE2);
  vector_elementwise_input_interface_dataw.push_back(BLOCKS_FF_AXIS_DATAW);
  vector_elementwise_input_interface_types.push_back(AXIS_TYPE0);
  vector_elementwise_input_interface_dataw.push_back(INST_AXIS_DATAW);
  vector_elementwise_input_interface_types.push_back(AXIS_TYPE0);
  vector_elementwise_input_interface_dataw.push_back(INST_AXIS_DATAW);

  ld_input_interface_types.push_back(AXIS_TYPE0);
  ld_input_interface_dataw.push_back(INST_AXIS_DATAW);
  ld_output_interface_types.push_back(AXIS_TYPE1);
  ld_output_interface_dataw.push_back(WB0_AXIS_DATAW);

  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    for (unsigned int block_id = 0; block_id < NUM_PIPELINE_BLOCKS - 1; block_id++) {
      inst_dispatcher_output_interface_types.push_back(AXIS_TYPE0);
      inst_dispatcher_output_interface_dataw.push_back(INST_AXIS_DATAW);
    }
  }
  inst_dispatcher_output_interface_types.push_back(AXIS_TYPE0);
  inst_dispatcher_output_interface_dataw.push_back(INST_AXIS_DATAW);

  unsigned int node_id;
  std::string node_name;

  node_name = "axis_mvu_sector_0";
  node_id = module_assignment[node_name];
  cout << node_name << ": " << node_id << endl;
  source_node_ids.push_back(node_id);
  sink_node_ids.push_back(node_id);
  source_node_periods.push_back(BASEDIE_PERIOD);
  source_node_clks.push_back(1);
  sink_node_clks.push_back(1);
  source_adapter_interface_types.push_back(first_sector_output_interface_types);
  sink_adapter_interface_types.push_back(first_sector_input_interface_types);
  source_adapter_interface_dataw.push_back(first_sector_output_interface_dataw);
  sink_adapter_interface_dataw.push_back(first_sector_input_interface_dataw);

  for (unsigned int sector_id = 1; sector_id < SECTORS; sector_id++) {
    node_name = "axis_mvu_sector_" + std::to_string(sector_id);
    node_id = module_assignment[node_name];
    cout << node_name << ": " << node_id << endl;
    source_node_ids.push_back(node_id);
    sink_node_ids.push_back(node_id);
    source_node_periods.push_back(BASEDIE_PERIOD);
    source_node_clks.push_back(1);
    sink_node_clks.push_back(1);
    source_adapter_interface_types.push_back(sector_output_interface_types);
    sink_adapter_interface_types.push_back(sector_input_interface_types);
    source_adapter_interface_dataw.push_back(sector_output_interface_dataw);
    sink_adapter_interface_dataw.push_back(sector_input_interface_dataw);
  }

  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
      node_name = "axis_vector_elementwise_" + std::to_string(thread_id) + "_" + std::to_string(sector_id);
      node_id = module_assignment[node_name];
      cout << node_name << ": " << node_id << endl;
      sink_node_ids.push_back(node_id);
      sink_node_clks.push_back(0);
      sink_adapter_interface_types.push_back(vector_elementwise_input_interface_types);
      sink_adapter_interface_dataw.push_back(vector_elementwise_input_interface_dataw);
    }

    node_name = "axis_ld_" + std::to_string(thread_id);
    node_id = module_assignment[node_name];
    cout << node_name << ": " << node_id << endl;
    source_node_ids.push_back(node_id);
    sink_node_ids.push_back(node_id);
    source_node_periods.push_back(FABRIC_PERIOD);
    source_node_clks.push_back(0);
    sink_node_clks.push_back(0);
    source_adapter_interface_types.push_back(ld_output_interface_types);
    sink_adapter_interface_types.push_back(ld_input_interface_types);
    source_adapter_interface_dataw.push_back(ld_output_interface_dataw);
    sink_adapter_interface_dataw.push_back(ld_input_interface_dataw);
  }

  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    node_name = "axis_inst_dispatcher_" + std::to_string(thread_id);
    node_id = module_assignment[node_name];
    cout << node_name << ": " << node_id << endl;
    source_node_ids.push_back(node_id);
    source_node_periods.push_back(FABRIC_PERIOD);
    source_node_clks.push_back(0);
    source_adapter_interface_types.push_back(inst_dispatcher_output_interface_types);
    source_adapter_interface_dataw.push_back(inst_dispatcher_output_interface_dataw);
  }
}

design::design(const sc_module_name &name, std::string &noc_config_filename, std::vector<sc_clock *> &node_clks)
    : sc_module(name),
      sector_chain_fifo_rdy_signals("sector_chain_fifo_rdy_signals"),
      sector_chain_fifo_ren_signals("sector_chain_fifo_ren_signals"),
      sector_chain_fifo_rdata_signals("sector_chain_fifo_rdata_signals"),
      mrf_waddr("mrf_waddr"),
      mrf_wdata("mrf_wdata"),
      mrf_wid("mrf_wid"),
      ififo_rdy("ififo_rdy"),
      ififo_wen("ififo_ren"),
      ififo_wdata("ififo_wdata"),
      ofifo_rdy("ofifo_rdy"),
      ofifo_ren("ofifo_ren"),
      ofifo_rdata("ofifo_rdata") {
  init_vector<sc_out<bool>>::init_sc_vector(ififo_rdy, THREADS, CORES);
  init_vector<sc_in<bool>>::init_sc_vector(ififo_wen, THREADS, CORES);
  init_vector<sc_in<data_vector<tb_input_precision>>>::init_sc_vector(ififo_wdata, THREADS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(ofifo_rdy, THREADS, CORES);
  init_vector<sc_in<bool>>::init_sc_vector(ofifo_ren, THREADS, CORES);
  init_vector<sc_out<data_vector<tb_output_precision>>>::init_sc_vector(ofifo_rdata, THREADS, CORES);

  init_vector<sc_signal<bool>>::init_sc_vector(sector_chain_fifo_rdy_signals, SECTORS, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(sector_chain_fifo_ren_signals, SECTORS, THREADS, CORES);
  init_vector<sc_signal<data_vector<tb_input_precision>>>::init_sc_vector(sector_chain_fifo_rdata_signals, SECTORS,
                                                                          THREADS, CORES);

  /*********************************************************/
  /* Instantiate adapter information and create system NoC */
  /*********************************************************/

  prepare_adapters_info();
  _system_noc = new noc("system_noc", noc_config_filename, node_clks, source_node_ids, source_adapter_interface_types,
                        source_node_periods, source_node_clks, source_adapter_interface_dataw, sink_node_ids,
                        sink_adapter_interface_types, sink_adapter_interface_dataw, sink_node_clks);
  _system_noc->noc_clk(noc_clk);
  _system_noc->adapter_clk(adapter_clk);
  _system_noc->rst(rst);

  /***************************************/
  /* Resize AXI-streaming signal vectors */
  /***************************************/

  int num_sinks = sink_node_ids.size();
  int num_sources = source_node_ids.size();
  master_axis_signals.resize(num_sinks);
  for (unsigned int sink_id = 0; sink_id < num_sinks; sink_id++) {
    master_axis_signals[sink_id].resize(sink_adapter_interface_types[sink_id].size());
  }
  slave_axis_signals.resize(num_sources);
  for (unsigned int src_id = 0; src_id < num_sources; src_id++) {
    slave_axis_signals[src_id].resize(source_adapter_interface_types[src_id].size());
  }

  /**********************************************************************/
  /* Instantiate system blocks and connect external interfaces (if any) */
  /**********************************************************************/

  mvu_sectors.resize(SECTORS - 1);
  vector_elementwise_blocks.resize(THREADS);
  inst_dispatcher.resize(THREADS);
  char module_name[NAME_LENGTH];
  std::string module_name_str;

  first_mvu_sector = new axis_mvu_sector("axis_mvu_sector_0", 0, SECTOR_INST_AXIS_DATAW, SECTOR_WB0_AXIS_DATAW,
                                         SECTOR_FF_AXIS_DATAW, SECTOR_WB0_AXIS_DATAW);
  first_mvu_sector->clk(*node_clks[1]);
  first_mvu_sector->rst(rst);
  first_mvu_sector->mrf_waddr(mrf_waddr);
  first_mvu_sector->mrf_wdata(mrf_wdata);
  first_mvu_sector->mrf_wid(mrf_wid);
  first_mvu_sector->sector_chain_ofifo_rdy(sector_chain_fifo_rdy_signals[0]);
  first_mvu_sector->sector_chain_ofifo_ren(sector_chain_fifo_ren_signals[0]);
  first_mvu_sector->sector_chain_ofifo_rdata(sector_chain_fifo_rdata_signals[0]);

  for (unsigned int sector_id = 1; sector_id < SECTORS; sector_id++) {
    module_name_str = "axis_mvu_sector_" + std::to_string(sector_id);
    std::strcpy(module_name, module_name_str.c_str());
    mvu_sectors[sector_id - 1] =
        new axis_mvu_sector_chain(module_name, sector_id, SECTOR_INST_AXIS_DATAW, SECTOR_WB0_AXIS_DATAW,
                                  SECTOR_FF_AXIS_DATAW, SECTOR_WB0_AXIS_DATAW);
    mvu_sectors[sector_id - 1]->clk(*node_clks[1]);
    mvu_sectors[sector_id - 1]->rst(rst);
    mvu_sectors[sector_id - 1]->mrf_waddr(mrf_waddr);
    mvu_sectors[sector_id - 1]->mrf_wdata(mrf_wdata);
    mvu_sectors[sector_id - 1]->mrf_wid(mrf_wid);
    mvu_sectors[sector_id - 1]->sector_chain_ofifo_rdy(sector_chain_fifo_rdy_signals[sector_id]);
    mvu_sectors[sector_id - 1]->sector_chain_ofifo_ren(sector_chain_fifo_ren_signals[sector_id]);
    mvu_sectors[sector_id - 1]->sector_chain_ofifo_rdata(sector_chain_fifo_rdata_signals[sector_id]);
    mvu_sectors[sector_id - 1]->wb_fifo_rdy(sector_chain_fifo_rdy_signals[sector_id - 1]);
    mvu_sectors[sector_id - 1]->wb_fifo_ren(sector_chain_fifo_ren_signals[sector_id - 1]);
    mvu_sectors[sector_id - 1]->wb_fifo_rdata(sector_chain_fifo_rdata_signals[sector_id - 1]);
  }

  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    module_name_str = "axis_inst_dispatcher_" + std::to_string(thread_id);
    std::strcpy(module_name, module_name_str.c_str());
    inst_dispatcher[thread_id] = new axis_inst_dispatch(module_name, INST_AXIS_DATAW, thread_id);
    inst_dispatcher[thread_id]->clk(*node_clks[0]);
    inst_dispatcher[thread_id]->rst(rst);
    inst_dispatcher[thread_id]->start_pc(start_pc);
    inst_dispatcher[thread_id]->end_pc(end_pc);
    inst_dispatcher[thread_id]->start(start);
    inst_dispatcher[thread_id]->inst_wdata(inst_wdata);
    inst_dispatcher[thread_id]->inst_waddr(inst_waddr);
    inst_dispatcher[thread_id]->inst_wen(inst_wen);

    module_name_str = "axis_vector_elementwise_blocks_" + std::to_string(thread_id);
    std::strcpy(module_name, module_name_str.c_str());
    vector_elementwise_blocks[thread_id] =
        new axis_vector_elementwise(module_name, INST_AXIS_DATAW, WB0_AXIS_DATAW, BLOCKS_FF_AXIS_DATAW, thread_id);
    vector_elementwise_blocks[thread_id]->clk(*node_clks[0]);
    vector_elementwise_blocks[thread_id]->rst(rst);
    vector_elementwise_blocks[thread_id]->ext_input_fifo_rdy(ififo_rdy[thread_id]);
    vector_elementwise_blocks[thread_id]->ext_input_fifo_wen(ififo_wen[thread_id]);
    vector_elementwise_blocks[thread_id]->ext_input_fifo_wdata(ififo_wdata[thread_id]);
    vector_elementwise_blocks[thread_id]->ext_output_fifo_rdy(ofifo_rdy[thread_id]);
    vector_elementwise_blocks[thread_id]->ext_output_fifo_ren(ofifo_ren[thread_id]);
    vector_elementwise_blocks[thread_id]->ext_output_fifo_rdata(ofifo_rdata[thread_id]);
  }

  /*********************************/
  /* Connect NoC source interfaces */
  /*********************************/

  unsigned int interface_id = 0;
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
      if (sector_id == 0)
        slave_axis_signals[sector_id][thread_id].Connect(first_mvu_sector->sector_ofifo_interface[thread_id],
                                                         _system_noc->noc_slave_ports[sector_id][thread_id]);
      else
        slave_axis_signals[sector_id][thread_id].Connect(mvu_sectors[sector_id - 1]->sector_ofifo_interface[thread_id],
                                                         _system_noc->noc_slave_ports[sector_id][thread_id]);
    }
    interface_id++;
  }

  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    slave_axis_signals[interface_id][0].Connect(vector_elementwise_blocks[thread_id]->loader_wb0_interface,
                                                _system_noc->noc_slave_ports[interface_id][0]);
    interface_id++;
  }

  unsigned int interface_num;
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
      interface_num = sector_id * (NUM_PIPELINE_BLOCKS - 1);
      cout << interface_num << endl;

      slave_axis_signals[interface_id][interface_num].Connect(
          inst_dispatcher[thread_id]->sector_mop_interface[sector_id],
          _system_noc->noc_slave_ports[interface_id][interface_num]);

      slave_axis_signals[interface_id][interface_num + 1].Connect(
          inst_dispatcher[thread_id]->evrf_mop_interface[sector_id],
          _system_noc->noc_slave_ports[interface_id][interface_num + 1]);

      slave_axis_signals[interface_id][interface_num + 2].Connect(
          inst_dispatcher[thread_id]->mfu0_mop_interface[sector_id],
          _system_noc->noc_slave_ports[interface_id][interface_num + 2]);

      slave_axis_signals[interface_id][interface_num + 3].Connect(
          inst_dispatcher[thread_id]->mfu1_mop_interface[sector_id],
          _system_noc->noc_slave_ports[interface_id][interface_num + 3]);
    }
    interface_num = SECTORS * (NUM_PIPELINE_BLOCKS - 1);
    cout << interface_num << endl;
    slave_axis_signals[interface_id][interface_num].Connect(inst_dispatcher[thread_id]->ld_mop_interface,
                                                            _system_noc->noc_slave_ports[interface_id][interface_num]);
    interface_id++;
  }
  cout << "Finished source interfaces" << endl;

  /*******************************/
  /* Connect NoC sink interfaces */
  /*******************************/

  interface_id = 0;
  for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
      if (sector_id == 0)
        master_axis_signals[interface_id][thread_id].Connect(_system_noc->noc_master_ports[interface_id][thread_id],
                                                             first_mvu_sector->sector_inst_interface[thread_id]);
      else
        master_axis_signals[interface_id][thread_id].Connect(
            _system_noc->noc_master_ports[interface_id][thread_id],
            mvu_sectors[sector_id - 1]->sector_inst_interface[thread_id]);
    }
    if (sector_id == 0) {
      for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
        master_axis_signals[interface_id][THREADS + thread_id].Connect(
            _system_noc->noc_master_ports[interface_id][THREADS + thread_id],
            first_mvu_sector->sector_wb_interface[thread_id]);
      }
    }
    interface_id++;
  }

  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    for (unsigned int sector_id = 0; sector_id < SECTORS; sector_id++) {
      master_axis_signals[interface_id][0].Connect(
          _system_noc->noc_master_ports[interface_id][0],
          vector_elementwise_blocks[thread_id]->evrf_inst_interfaces[sector_id]);
      master_axis_signals[interface_id][1].Connect(
          _system_noc->noc_master_ports[interface_id][1],
          vector_elementwise_blocks[thread_id]->evrf_ififo_interfaces[sector_id]);

      master_axis_signals[interface_id][2].Connect(
          _system_noc->noc_master_ports[interface_id][2],
          vector_elementwise_blocks[thread_id]->mfu0_inst_interfaces[sector_id]);

      master_axis_signals[interface_id][3].Connect(
          _system_noc->noc_master_ports[interface_id][3],
          vector_elementwise_blocks[thread_id]->mfu1_inst_interfaces[sector_id]);
      interface_id++;
    }
    master_axis_signals[interface_id][0].Connect(_system_noc->noc_master_ports[interface_id][0],
                                                 vector_elementwise_blocks[thread_id]->loader_inst_interface);
    interface_id++;
  }
}

design::~design() {
  delete _system_noc;
  delete first_mvu_sector;
  for (unsigned int sector_id = 1; sector_id < SECTORS; sector_id++) delete mvu_sectors[sector_id - 1];
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    delete vector_elementwise_blocks[thread_id];
    delete inst_dispatcher[thread_id];
  }
}