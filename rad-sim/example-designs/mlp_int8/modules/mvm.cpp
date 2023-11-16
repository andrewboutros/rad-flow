#include "mvm.hpp"

mvm::mvm(const sc_module_name &name, unsigned int id_mvm, unsigned int id_layer)
    : RADSimModule(name),
      rf_rdata("rf_rdata", DPES),
      rf_wdata("rf_wdata"),
      rf_wen("rf_wen", DPES),
      rf_raddr("rf_raddr"),
      rf_waddr("rf_waddr"),
      rf_clk_en("rf_clk_en"),
      datapath_reduction_operand("datapath_reduction_operand", DPES),
      datapath_ovalid("datapath_ovalid", DPES),
      datapath_results("datapath_results", DPES),
      tuser_rf_en("tuser_rf_en", DPES) {

  module_name = name;
  mvm_id = id_mvm;
  layer_id = id_layer;

  char fifo_name[50];
  std::string fifo_name_str;
  char pipeline_name[50];
  std::string pipeline_name_str;
  char mem_name[50];
  std::string mem_name_str;
  char datapath_name[50];
  std::string datapath_name_str;
  rf.resize(DPES);
  datapath_inst.resize(DPES);
  std::string mvm_dir = radsim_config.GetStringKnob("radsim_user_design_root_dir");
  std::string mem_init_file;

  // STAGE 1: Instruction FIFO, Input FIFO, and Reduction FIFO
  
  fifo_name_str = "layer" + std::to_string(layer_id) + "mvm" + std::to_string(mvm_id) + "_inst_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  instruction_fifo = new fifo<mvm_inst>(fifo_name, FIFOD, FIFOD - 4);
  instruction_fifo->clk(clk);
  instruction_fifo->rst(rst);
  instruction_fifo->push(inst_fifo_push);
  instruction_fifo->pop(inst_fifo_pop);
  instruction_fifo->idata(inst_fifo_idata);
  instruction_fifo->odata(inst_fifo_odata);
  instruction_fifo->empty(inst_fifo_empty);
  instruction_fifo->full(inst_fifo_full);
  instruction_fifo->almost_full(inst_fifo_almost_full);

  fifo_name_str = "layer" + std::to_string(layer_id) + "mvm" + std::to_string(mvm_id) + "_input_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  input_fifo = new fifo<data_vector<sc_int<IPRECISION>>>(fifo_name, FIFOD, FIFOD - 4);
  input_fifo->clk(clk);
  input_fifo->rst(rst);
  input_fifo->push(input_fifo_push);
  input_fifo->pop(input_fifo_pop);
  input_fifo->idata(input_fifo_idata);
  input_fifo->odata(input_fifo_odata);
  input_fifo->empty(input_fifo_empty);
  input_fifo->full(input_fifo_full);
  input_fifo->almost_full(input_fifo_almost_full);
  
  fifo_name_str = "layer" + std::to_string(layer_id) + "mvm" + std::to_string(mvm_id) + "_reduction_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  reduction_fifo =
      new fifo<data_vector<sc_int<IPRECISION>>>(fifo_name, FIFOD, FIFOD - 4);
  reduction_fifo->clk(clk);
  reduction_fifo->rst(rst);
  reduction_fifo->push(reduction_fifo_push);
  reduction_fifo->pop(reduction_fifo_pop);
  reduction_fifo->idata(reduction_fifo_idata);
  reduction_fifo->odata(reduction_fifo_odata);
  reduction_fifo->empty(reduction_fifo_empty);
  reduction_fifo->full(reduction_fifo_full);
  reduction_fifo->almost_full(reduction_fifo_almost_full);

  // STAGE 2: Weight RFs, Instruction/Valid/Input/Reduction Pipelines

  for (unsigned int dot_id = 0; dot_id < DPES; dot_id++) {
    mem_name_str = "layer" + std::to_string(layer_id) + 
        "mvm" + std::to_string(mvm_id) + "_rf" + std::to_string(dot_id);
    std::strcpy(mem_name, mem_name_str.c_str());
    rf[dot_id] = new register_file<sc_int<IPRECISION>>(
        mem_name, dot_id, RF_DEPTH, LANES, mem_init_file);
    rf[dot_id]->clk(clk);
    rf[dot_id]->rst(rst);
    rf[dot_id]->raddr(inst_rf_raddr);
    rf[dot_id]->wdata(rf_wdata);
    rf[dot_id]->waddr(rf_waddr);
    rf[dot_id]->wen(rf_wen[dot_id]);
    rf[dot_id]->clk_en(rf_clk_en);
    rf[dot_id]->rdata(rf_rdata[dot_id]);
  }

  pipeline_name_str = "layer" + std::to_string(layer_id) + "mvm" + std::to_string(mvm_id) + "_inst_rf_pipeline";
  std::strcpy(pipeline_name, pipeline_name_str.c_str());
  inst_rf_pipeline = new pipeline<mvm_inst>(pipeline_name, RF_RD_LATENCY);
  inst_rf_pipeline->clk(clk);
  inst_rf_pipeline->rst(rst);
  inst_rf_pipeline->idata(inst_fifo_odata);
  inst_rf_pipeline->odata(inst_rf_pipeline_odata);

  pipeline_name_str = "layer" + std::to_string(layer_id) + "mvm" + std::to_string(mvm_id) + "_inst_valid_rf_pipeline";
  std::strcpy(pipeline_name, pipeline_name_str.c_str());
  inst_valid_rf_pipeline = new pipeline<bool>(pipeline_name, RF_RD_LATENCY);
  inst_valid_rf_pipeline->clk(clk);
  inst_valid_rf_pipeline->rst(rst);
  inst_valid_rf_pipeline->idata(inst_fifo_pop);
  inst_valid_rf_pipeline->odata(inst_rf_pipeline_valid);

  pipeline_name_str = "layer" + std::to_string(layer_id) + "mvm" + std::to_string(mvm_id) + "_input_pipeline";
  std::strcpy(pipeline_name, pipeline_name_str.c_str());
  input_pipeline = new pipeline<data_vector<sc_int<IPRECISION>>>(pipeline_name, RF_RD_LATENCY);
  input_pipeline->clk(clk);
  input_pipeline->rst(rst);
  input_pipeline->idata(input_fifo_odata);
  input_pipeline->odata(input_pipeline_odata);

  pipeline_name_str = "layer" + std::to_string(layer_id) + "mvm" + std::to_string(mvm_id) + "_reduction_pipeline";
  std::strcpy(pipeline_name, pipeline_name_str.c_str());
  reduction_pipeline = new pipeline<data_vector<sc_int<IPRECISION>>>(pipeline_name, RF_RD_LATENCY);
  reduction_pipeline->clk(clk);
  reduction_pipeline->rst(rst);
  reduction_pipeline->idata(reduction_fifo_odata);
  reduction_pipeline->odata(reduction_pipeline_odata);

  // STAGE 3: Compute Datapaths, Instruction pipeline

  for (unsigned int dot_id = 0; dot_id < DPES; dot_id++) {
    datapath_name_str = "layer" + std::to_string(layer_id) + 
        "mvm" + std::to_string(mvm_id) + "_datapath" + std::to_string(dot_id);
    std::strcpy(datapath_name, datapath_name_str.c_str());
    datapath_inst[dot_id] = new datapath(datapath_name, layer_id, mvm_id, dot_id);
    datapath_inst[dot_id]->clk(clk);
    datapath_inst[dot_id]->rst(rst);
    datapath_inst[dot_id]->ivalid(inst_rf_pipeline_valid);
    datapath_inst[dot_id]->dataa(input_pipeline_odata);               // input operand
    datapath_inst[dot_id]->datab(rf_rdata[dot_id]);                   // rf operand
    datapath_inst[dot_id]->datac(datapath_reduction_operand[dot_id]); // reduction operand
    datapath_inst[dot_id]->accum_addr(datapath_inst_accum);
    datapath_inst[dot_id]->accum(datapath_inst_accum_en);
    datapath_inst[dot_id]->last(datapath_inst_release);
    datapath_inst[dot_id]->reduce(datapath_inst_reduce);
    datapath_inst[dot_id]->ovalid(datapath_ovalid[dot_id]);
    datapath_inst[dot_id]->oresult(datapath_results[dot_id]);
  }  

  pipeline_name_str = "layer" + std::to_string(layer_id) + "mvm" + std::to_string(mvm_id) + "_inst_datapath_pipeline";
  std::strcpy(pipeline_name, pipeline_name_str.c_str());
  inst_datapath_pipeline = new pipeline<mvm_inst>(pipeline_name, DATAPATH_DELAY);
  inst_datapath_pipeline->clk(clk);
  inst_datapath_pipeline->rst(rst);
  inst_datapath_pipeline->idata(inst_rf_pipeline_odata);
  inst_datapath_pipeline->odata(inst_datapath_pipeline_odata);

  // STAGE 4: Output FIFO

  fifo_name_str = "layer" + std::to_string(layer_id) + "mvm" + std::to_string(mvm_id) + "_output_data_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  output_data_fifo =
      new fifo<data_vector<sc_int<IPRECISION>>>(fifo_name, FIFOD, FIFOD - DATAPATH_DELAY - 4);
  output_data_fifo->clk(clk);
  output_data_fifo->rst(rst);
  output_data_fifo->push(output_data_fifo_push);
  output_data_fifo->pop(output_data_fifo_pop);
  output_data_fifo->idata(output_data_fifo_idata);
  output_data_fifo->odata(output_data_fifo_odata);
  output_data_fifo->full(output_data_fifo_full);
  output_data_fifo->almost_full(output_data_fifo_almost_full);
  output_data_fifo->empty(output_data_fifo_empty);

  SC_METHOD(Assign);
  sensitive << rst << inst_fifo_odata << rx_interface.tuser << inst_fifo_empty << input_fifo_empty
    << reduction_fifo_empty << output_data_fifo_almost_full << inst_rf_pipeline_odata << datapath_ovalid[0]
    << inst_datapath_pipeline_odata << output_data_fifo_odata << output_data_fifo_empty << rx_interface.tvalid
    << inst_fifo_almost_full << input_fifo_almost_full << reduction_fifo_almost_full << tx_interface.tready
    << reduction_pipeline_odata;
  for (unsigned int i = 0; i < DPES; i++) sensitive << datapath_results[i];
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);

  this->RegisterModuleInfo();
}

mvm::~mvm() { 
  delete instruction_fifo;
  delete inst_rf_pipeline;
  delete inst_valid_rf_pipeline;
  delete input_fifo;
  delete reduction_fifo;
  delete input_pipeline;
  delete reduction_pipeline;
  for (unsigned int i = 0; i < DPES; i++) {
    delete rf[i];
    delete datapath_inst[i];
  }
  delete inst_datapath_pipeline;
  delete output_data_fifo;
}

void mvm::Tick() {
  // Reset logic
  input_fifo_push.write(false);
  reduction_fifo_push.write(false);
  for (unsigned int i = 0; i < DPES; i++) rf_wen[i].write(false);
  wait();

  // Sequential logic
  while (true) {
    // Read from input interface and steer to destination mem/FIFO
    if (rx_interface.tvalid.read() && rx_interface.tready.read()) {
      sc_bv<AXIS_MAX_DATAW> tdata = rx_interface.tdata.read();
      data_vector<sc_int<IPRECISION>> temp_data_vec(LANES);
      
      if (tuser_op.read() == 0) {
        // std::cout << "(" << layer_id << "," << mvm_id << "):" << "GOT INST" << std::endl;
        // If new input is an instruction
        mvm_inst inst;
        inst.from_bv(rx_interface.tdata.read());
        inst_fifo_idata.write(inst);
        inst_fifo_push.write(true);
        input_fifo_push.write(false);
        reduction_fifo_push.write(false);
        for (unsigned int i = 0; i < DPES; i++) rf_wen[i].write(false);
      } else if (tuser_op.read() == 1) {
        // std::cout << "(" << layer_id << "," << mvm_id << "):" "GOT REDUCE" << std::endl;
        // If new input is a reduction vector
        for (unsigned int lane_id = 0; lane_id < LANES; lane_id++) {
          temp_data_vec[lane_id] =
              tdata.range((lane_id + 1) * IPRECISION - 1, lane_id * IPRECISION).to_int();
        }
        reduction_fifo_idata.write(temp_data_vec);
        inst_fifo_idata.write(inst_fifo_odata.read());
        inst_fifo_push.write(inst_fifo_pop.read());
        input_fifo_push.write(false);
        reduction_fifo_push.write(true);
        for (unsigned int i = 0; i < DPES; i++) rf_wen[i].write(false);
      } else if (tuser_op.read() == 2) {
        // std::cout << "(" << layer_id << "," << mvm_id << "):" "GOT INPUT" << std::endl;
        // If new input is an input vector
        for (unsigned int lane_id = 0; lane_id < LANES; lane_id++) {
          temp_data_vec[lane_id] =
              tdata.range((lane_id + 1) * IPRECISION - 1, lane_id * IPRECISION).to_int();
        }
        input_fifo_idata.write(temp_data_vec);
        inst_fifo_idata.write(inst_fifo_odata.read());
        inst_fifo_push.write(inst_fifo_pop.read());
        input_fifo_push.write(true);
        reduction_fifo_push.write(false);
        for (unsigned int i = 0; i < DPES; i++) rf_wen[i].write(false);
      } else if (tuser_op.read() == 3) {
        // std::cout << "GOT W" << std::endl;
        // If new input is an RF word
        for (unsigned int lane_id = 0; lane_id < LANES; lane_id++) {
          temp_data_vec[lane_id] =
              tdata.range((lane_id + 1) * IPRECISION - 1, lane_id * IPRECISION).to_int();
        }
        rf_wdata.write(temp_data_vec);
        rf_waddr.write(tuser_rf_addr);
        inst_fifo_idata.write(inst_fifo_odata.read());
        inst_fifo_push.write(inst_fifo_pop.read());
        input_fifo_push.write(false);
        reduction_fifo_push.write(false);
        for (unsigned int i = 0; i < DPES; i++) {
          if (tuser_rf_en[i] == 1) { 
            rf_wen[i].write(true);
          } else {
            rf_wen[i].write(false);
          }
        }
      } else {
        inst_fifo_idata.write(inst_fifo_odata.read());
        inst_fifo_push.write(inst_fifo_pop.read());
        input_fifo_push.write(false);
        reduction_fifo_push.write(false);
        for (unsigned int i = 0; i < DPES; i++) rf_wen[i].write(false);
      }
    } else {
      inst_fifo_idata.write(inst_fifo_odata.read());
      inst_fifo_push.write(inst_fifo_pop.read());
      input_fifo_push.write(false);
      reduction_fifo_push.write(false);
      for (unsigned int i = 0; i < DPES; i++) rf_wen[i].write(false);
    }
    wait();
  }
}

void mvm::Assign() {
  if (rst.read()) {
    // Module signals
    inst_rf_raddr.write(0);
    rf_clk_en.write(false);
    tuser_rf_addr.write(0);
    tuser_op.write(0);
    for(unsigned int i = 0; i < DPES; i++) tuser_rf_en[i].write(false); 
    //inst_fifo_push.write(false);   
    inst_fifo_pop.write(false);
    input_fifo_pop.write(false);
    reduction_fifo_pop.write(false);
    datapath_inst_accum.write(0);
    datapath_inst_accum_en.write(false);
    datapath_inst_reduce.write(false);
    datapath_inst_release.write(false);
    output_data_fifo_push.write(false);

    // Interface signals
    rx_interface.tready.write(false);
    tx_interface.tdata.write(0);
    tx_interface.tvalid.write(false);
    tx_interface.tstrb.write((2 << AXIS_STRBW) - 1);
    tx_interface.tkeep.write((2 << AXIS_KEEPW) - 1);
    tx_interface.tlast.write(0);
    tx_interface.tuser.write(0);
  } else {
    // Split the instructions into fields for ease-of-use later
    inst_rf_raddr.write(inst_fifo_odata.read().rf_raddr);

    // Split the tuser field for ease-of-use later
    tuser_rf_addr.write(rx_interface.tuser.read().range(8, 0).to_uint());
    tuser_op.write(rx_interface.tuser.read().range(10, 9).to_uint());
    for(unsigned int i = 0; i < DPES; i++) tuser_rf_en[i].write(rx_interface.tuser.read().range(11+i, 11+i). to_uint());

    // Process next instruction if there is an instruction and input vector available, and the output FIFO is able to take outputs
    inst_fifo_pop.write(!inst_fifo_empty.read() && !input_fifo_empty.read() && !output_data_fifo_almost_full.read()
      && (!inst_fifo_odata.read().reduce || !reduction_fifo_empty.read()));
    // Pop reduction vector if a request to reduce is made, the reduction vector is available, and the next instruction is able to be processed
    reduction_fifo_pop.write(!inst_fifo_empty.read() && !input_fifo_empty.read() && !output_data_fifo_almost_full.read()
      && inst_fifo_odata.read().reduce && !reduction_fifo_empty.read());
    // Pop input vector if this is the last chunk for the input vector, and the next instruction is able to be processed
    input_fifo_pop.write(!inst_fifo_empty.read() && !input_fifo_empty.read() && !output_data_fifo_almost_full.read()
      && (!inst_fifo_odata.read().reduce || !reduction_fifo_empty.read()) && inst_fifo_odata.read().last);
    
    // Split the instructions into fields for ease-of-use later
    datapath_inst_accum.write(inst_rf_pipeline_odata.read().accum_raddr);
    datapath_inst_accum_en.write(inst_rf_pipeline_odata.read().accum_en);
    datapath_inst_reduce.write(inst_rf_pipeline_odata.read().reduce);
    datapath_inst_release.write(inst_rf_pipeline_odata.read().release);

    // Split reduction operands
    data_vector<sc_int<IPRECISION>> reduction_data = reduction_pipeline_odata.read();
    if (reduction_data.size() > 0) {
      for (unsigned int i = 0; i < DPES; i++) {
        datapath_reduction_operand[i].write(reduction_data[i]);
      }
    }

    // Concatenate datapath results, destination id, and op (input/reduction) to write to output FIFO
    data_vector<sc_int<IPRECISION>> truncated_datapath_results(DPES+2);
    for (unsigned int i = 0; i < DPES; i++) {
      truncated_datapath_results[i] = datapath_results[i].read();
    }
    truncated_datapath_results[DPES] = inst_datapath_pipeline_odata.read().release_dest;
    truncated_datapath_results[DPES+1] = inst_datapath_pipeline_odata.read().release_op;
    output_data_fifo_idata.write(truncated_datapath_results);
    output_data_fifo_push.write(datapath_ovalid[0]);

    // Output data FIFO
    data_vector<sc_int<IPRECISION>> output_data_fifo_elements = output_data_fifo_odata.read();
    if (output_data_fifo_elements.size() > 0) {
      sc_bv<AXIS_MAX_DATAW> tx_tdata_bv;
      for (unsigned int lane_id = 0; lane_id < LANES; lane_id++) {
        tx_tdata_bv.range((lane_id + 1) * IPRECISION - 1, lane_id * IPRECISION) =
            output_data_fifo_elements[lane_id];
      }
      tx_interface.tdata.write(tx_tdata_bv);
      // Send tuser field as either input or reduction vector
      sc_bv<AXIS_USERW> tx_tuser_bv;
      if (output_data_fifo_elements[DPES+1]) {
        tx_tuser_bv.range(10, 9) = 2; // input
      } else {
        tx_tuser_bv.range(10, 9) = 1; // reduction
      }
      tx_interface.tuser.write(tx_tuser_bv);
      tx_interface.tdest.write(output_data_fifo_elements[DPES]);
      tx_interface.tvalid.write(!output_data_fifo_empty.read());
    } else {
      tx_interface.tvalid.write(false);
    }
    output_data_fifo_pop.write(!output_data_fifo_empty.read() && tx_interface.tready.read());

    // Specify if ready to accept input
    if (rx_interface.tvalid.read() && rx_interface.tuser.read().range(10, 9).to_uint() == 0) { // Inst memory
      rx_interface.tready.write(!inst_fifo_almost_full.read());
    } else if (rx_interface.tvalid.read() && rx_interface.tuser.read().range(10, 9).to_uint() == 1) { // Reduction FIFO
      rx_interface.tready.write(!reduction_fifo_almost_full.read());
    } else if (rx_interface.tvalid.read() && rx_interface.tuser.read().range(10, 9).to_uint() == 2) { // Input FIFO
      rx_interface.tready.write(!input_fifo_almost_full.read());
    } else if (rx_interface.tvalid.read() && rx_interface.tuser.read().range(10, 9).to_uint() == 3) { // Matrix memory
      rx_interface.tready.write(true);
    } else {
      rx_interface.tready.write(false);
    }
  }
}

void mvm::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;

  port_name = module_name + ".tx_interface";
  RegisterAxisMasterPort(port_name, &tx_interface, DATAW, 0);

  port_name = module_name + ".rx_interface";
  RegisterAxisSlavePort(port_name, &rx_interface, DATAW, 0);
}
