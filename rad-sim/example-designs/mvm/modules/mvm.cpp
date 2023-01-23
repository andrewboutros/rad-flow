#include "mvm.hpp"

bool ParseInstructions(std::vector<mvm_inst>& inst_mem, const std::string& inst_filename) {
  std::ifstream inst_file(inst_filename);
  if(!inst_file)
    return false;

  std::string line;
  uint32_t addr = 0;
  while(std::getline(inst_file, line)) {
    std::stringstream line_stream(line);
    mvm_inst inst;
    uint32_t value;
    line_stream >> value;
    inst.en = value;
    line_stream >> value;
    inst.jump = value;
    line_stream >> value;
    inst.reduce = value;
    line_stream >> value;
    inst.accum = value;
    line_stream >> value;
    inst.accum_en = value;
    line_stream >> value;
    inst.release = value;
    line_stream >> value;
    inst.raddr = value;
    line_stream >> value;
    inst.last = value;
    line_stream >> value;
    inst.dest_layer = value;
    line_stream >> value;
    inst.dest_mvm = value;
    inst_mem[addr] = inst;
    addr++;
  }
  return true;
}

mvm::mvm(const sc_module_name& name, unsigned int id_mvm, unsigned int id_layer, const std::string& inst_filename) : 
  radsim_module(name),
  matrix_mem_rdata("matrix_mem_rdata", DOT_PRODUCTS),
  matrix_mem_wen("matrix_mem_wen", DOT_PRODUCTS),
  ififo_pipeline("ififo_pipeline", RF_RD_LATENCY),
  reduce_pipeline("reduce_pipeline", RF_RD_LATENCY),
  result_pipeline("result_pipeline", COMPUTE_LATENCY),
  valid_pipeline("valid_pipeline", COMPUTE_LATENCY+RF_RD_LATENCY),
  release_pipeline("release_pipeline", RF_RD_LATENCY),
  accum_en_pipeline("accum_en_pipeline", RF_RD_LATENCY),
  accum_pipeline("accum_pipeline", RF_RD_LATENCY),
  dest_layer_pipeline("dest_layer_pipeline", COMPUTE_LATENCY+RF_RD_LATENCY),
  dest_mvm_pipeline("mvm_layer_pipeline", COMPUTE_LATENCY+RF_RD_LATENCY),
  tdata_vec(LANES),
  result(DOT_PRODUCTS),
  rst("rst") {
  
  module_name = name;
  mvm_id = id_mvm;
  layer_id = id_layer;

  inst_memory.resize(MEM_DEPTH);
  if (!inst_filename.empty()) {
    if (!ParseInstructions(inst_memory, inst_filename)) {
      std::cerr << "Parsing instructions failed!" << std::endl;
      exit(1);
    }
  }
  accum_memory.resize(MEM_DEPTH);

  char mem_name[25];
  std::string mem_name_str;
  matrix_memory.resize(DOT_PRODUCTS);
  std::string mvm_dir = radsim_config.GetStringKnob("radsim_user_design_root_dir");
  std::string mem_init_file;
  for (unsigned int dot_id = 0; dot_id < DOT_PRODUCTS; dot_id++) {
    mem_init_file = mvm_dir + "/compiler/weight_mifs/layer" 
      + std::to_string(layer_id) + "_mvm" + std::to_string(mvm_id) + "_dot" + std::to_string(dot_id) + ".mif";
    mem_name_str = "mvm" + std::to_string(mvm_id) + "_matrix_mem" + std::to_string(dot_id);
    std::strcpy(mem_name, mem_name_str.c_str());
    matrix_memory[dot_id] = new register_file<sc_int<32>>(mem_name, dot_id, MEM_DEPTH, LANES, mem_init_file);
    matrix_memory[dot_id]->clk(clk);
    matrix_memory[dot_id]->rst(rst);
    matrix_memory[dot_id]->raddr(matrix_mem_raddr);
    matrix_memory[dot_id]->wdata(matrix_mem_wdata);
    matrix_memory[dot_id]->waddr(matrix_mem_waddr);
    matrix_memory[dot_id]->wen(matrix_mem_wen[dot_id]);
    matrix_memory[dot_id]->clk_en(matrix_mem_clk_en);
    matrix_memory[dot_id]->rdata(matrix_mem_rdata[dot_id]);
  }

  char fifo_name[25];
  std::string fifo_name_str;
  fifo_name_str = "mvm" + std::to_string(mvm_id) + "_ififo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  ififo = new fifo<sc_int<32>>(fifo_name, FIFO_SIZE, LANES, FIFO_SIZE-4, 0);
  ififo->clk(clk);
  ififo->rst(rst);
  ififo->wen(ififo_wen_signal);
  ififo->ren(ififo_ren_signal);
  ififo->wdata(ififo_wdata_signal);
  ififo->full(ififo_full_signal);
  ififo->almost_full(ififo_almost_full_signal);
  ififo->empty(ififo_empty_signal);
  ififo->almost_empty(ififo_almost_empty_signal);
  ififo->rdata(ififo_rdata_signal);

  fifo_name_str = "mvm" + std::to_string(mvm_id) + "_reduce_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  reduce_fifo = new fifo<sc_int<32>>(fifo_name, FIFO_SIZE, LANES, FIFO_SIZE-4, 0);
  reduce_fifo->clk(clk);
  reduce_fifo->rst(rst);
  reduce_fifo->wen(reduce_fifo_wen_signal);
  reduce_fifo->ren(reduce_fifo_ren_signal);
  reduce_fifo->wdata(reduce_fifo_wdata_signal);
  reduce_fifo->full(reduce_fifo_full_signal);
  reduce_fifo->almost_full(reduce_fifo_almost_full_signal);
  reduce_fifo->empty(reduce_fifo_empty_signal);
  reduce_fifo->almost_empty(reduce_fifo_almost_empty_signal);
  reduce_fifo->rdata(reduce_fifo_rdata_signal);
  
  fifo_name_str = "mvm" + std::to_string(mvm_id) + "_ofifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  ofifo = new fifo<sc_int<32>>(fifo_name, FIFO_SIZE, LANES, FIFO_SIZE - COMPUTE_LATENCY - RF_RD_LATENCY - 4, 0);
  ofifo->clk(clk);
  ofifo->rst(rst);
  ofifo->wen(ofifo_wen_signal);
  ofifo->ren(ofifo_ren_signal);
  ofifo->wdata(ofifo_wdata_signal);
  ofifo->full(ofifo_full_signal);
  ofifo->almost_full(ofifo_almost_full_signal);
  ofifo->empty(ofifo_empty_signal);
  ofifo->almost_empty(ofifo_almost_empty_signal);
  ofifo->rdata(ofifo_rdata_signal);

  fifo_name_str = "mvm" + std::to_string(mvm_id) + "_dl_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  dl_fifo = new fifo<sc_int<5>>(fifo_name, FIFO_SIZE, 1, FIFO_SIZE - COMPUTE_LATENCY - RF_RD_LATENCY - 4, 0);
  dl_fifo->clk(clk);
  dl_fifo->rst(rst);
  dl_fifo->wen(dl_fifo_wen_signal);
  dl_fifo->ren(dl_fifo_ren_signal);
  dl_fifo->wdata(dl_fifo_wdata_signal);
  dl_fifo->full(dl_fifo_full_signal);
  dl_fifo->almost_full(dl_fifo_almost_full_signal);
  dl_fifo->empty(dl_fifo_empty_signal);
  dl_fifo->almost_empty(dl_fifo_almost_empty_signal);
  dl_fifo->rdata(dl_fifo_rdata_signal);

  fifo_name_str = "mvm" + std::to_string(mvm_id) + "_dm_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  dm_fifo = new fifo<sc_uint<5>>(fifo_name, FIFO_SIZE, 1, FIFO_SIZE - COMPUTE_LATENCY - RF_RD_LATENCY - 4, 0);
  dm_fifo->clk(clk);
  dm_fifo->rst(rst);
  dm_fifo->wen(dm_fifo_wen_signal);
  dm_fifo->ren(dm_fifo_ren_signal);
  dm_fifo->wdata(dm_fifo_wdata_signal);
  dm_fifo->full(dm_fifo_full_signal);
  dm_fifo->almost_full(dm_fifo_almost_full_signal);
  dm_fifo->empty(dm_fifo_empty_signal);
  dm_fifo->almost_empty(dm_fifo_almost_empty_signal);
  dm_fifo->rdata(dm_fifo_rdata_signal);

  SC_METHOD(Assign);
  sensitive << rst << ofifo_almost_full_signal << ofifo_rdata_signal << tx_interface.tvalid 
    << tx_interface.tready << rx_interface.tuser << ififo_almost_full_signal 
    << reduce_fifo_almost_full_signal << result_pipeline[COMPUTE_LATENCY-1]
    << valid_pipeline[RF_RD_LATENCY+COMPUTE_LATENCY-1] << ififo_empty_signal << reduce_fifo_empty_signal
    << next_inst << pc << dl_fifo_rdata_signal << dm_fifo_rdata_signal
    << dest_layer_pipeline[RF_RD_LATENCY+COMPUTE_LATENCY-1] << dest_mvm_pipeline[RF_RD_LATENCY+COMPUTE_LATENCY-1]
    << dl_fifo_rdata_signal << dm_fifo_rdata_signal;
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);

  this->RegisterModuleInfo();
}

mvm::~mvm() { 
  delete ofifo;
}

sc_int<32> dot(data_vector<sc_int<32>> v1, data_vector<sc_int<32>> v2) {
  sc_int<32> res = 0;
  for (unsigned int element_id = 0; element_id < v1.size(); element_id++) {
    res += (v1[element_id] * v2[element_id]);
  }
  return res;
}

void mvm::Tick() {
  // Reset logic
  for (unsigned int lane_id = 0; lane_id < LANES; lane_id++) {
    tdata_vec[lane_id] = 0;
  }
  pc.write(0);
  mvm_inst rst_inst;
  //next_inst.write(rst_inst);
  wait();
  // Sequential logic
  while(true) {
    // Instruction issue logic
    //next_inst.write(inst_memory[pc.read()]);
    
    // Compute logic
    if (dot_reduce_op) {
      ififo_pipeline[0].write(ififo_rdata_signal.read());
      reduce_pipeline[0].write(reduce_fifo_rdata_signal.read());
      //std::cout << "Dot-Reduce op @ MVM (" << layer_id << ", " << mvm_id << ")" << std::endl;
      valid_pipeline[0].write(true);
      accum_pipeline[0].write(next_inst.read().accum);
      accum_en_pipeline[0].write(next_inst.read().accum_en);
      release_pipeline[0].write(next_inst.read().release);
      dest_layer_pipeline[0].write(next_inst.read().dest_layer);
      dest_mvm_pipeline[0].write(next_inst.read().dest_mvm);
      pc.write(pc.read() + 1);
    } else if (dot_op) {
      //std::cout << "Dot op @ MVM (" << layer_id << ", " << mvm_id << ")" << std::endl;
      data_vector<sc_int<32>> zeros(LANES);
      ififo_pipeline[0].write(ififo_rdata_signal.read());
      reduce_pipeline[0].write(zeros);
      valid_pipeline[0].write(true);
      accum_pipeline[0].write(next_inst.read().accum);
      accum_en_pipeline[0].write(next_inst.read().accum_en);
      release_pipeline[0].write(next_inst.read().release);
      dest_layer_pipeline[0].write(next_inst.read().dest_layer);
      dest_mvm_pipeline[0].write(next_inst.read().dest_mvm);
      pc.write(pc.read() + 1);
    } else if (next_inst.read().en && next_inst.read().jump) {
      valid_pipeline[0].write(false);
      pc.write(next_inst.read().raddr);
    } else {
      valid_pipeline[0].write(false);
    }

    if (valid_pipeline[RF_RD_LATENCY-1].read()) {
      data_vector<sc_int<32>> reduce_vector = reduce_pipeline[RF_RD_LATENCY-1].read();
      uint32_t accum_addr = accum_pipeline[RF_RD_LATENCY-1].read();
      bool accum_en = accum_en_pipeline[RF_RD_LATENCY-1].read();
      data_vector<sc_int<32>> accum_operand = accum_memory[accum_addr];

      for (unsigned int dot_id = 0; dot_id < DOT_PRODUCTS; dot_id++) {
        result[dot_id] = dot(ififo_pipeline[RF_RD_LATENCY-1].read(), matrix_mem_rdata[dot_id].read()); 
        result[dot_id] += reduce_vector[dot_id];
        if (accum_en) {
          result[dot_id] += accum_operand[dot_id];
        }
      }
      accum_memory[accum_addr] = result;
      result_pipeline[0].write(result);
      //std::cout << "Result: " << result << std::endl;
    }

    // Advance pipelines
    for (unsigned int stage_id = 1; stage_id < RF_RD_LATENCY; stage_id++) {
      ififo_pipeline[stage_id].write(ififo_pipeline[stage_id-1].read());
      reduce_pipeline[stage_id].write(reduce_pipeline[stage_id-1].read());
      valid_pipeline[stage_id].write(valid_pipeline[stage_id-1].read());
      accum_pipeline[stage_id].write(accum_pipeline[stage_id-1].read());
      accum_en_pipeline[stage_id].write(accum_en_pipeline[stage_id-1].read());
      release_pipeline[stage_id].write(release_pipeline[stage_id-1].read());
      dest_layer_pipeline[stage_id].write(dest_layer_pipeline[stage_id-1].read());
      dest_mvm_pipeline[stage_id].write(dest_mvm_pipeline[stage_id-1].read());
    }
    valid_pipeline[RF_RD_LATENCY].write(valid_pipeline[RF_RD_LATENCY-1].read()
      && release_pipeline[RF_RD_LATENCY-1].read());
    dest_layer_pipeline[RF_RD_LATENCY].write(dest_layer_pipeline[RF_RD_LATENCY-1].read());
    dest_mvm_pipeline[RF_RD_LATENCY].write(dest_mvm_pipeline[RF_RD_LATENCY-1].read());
    for (unsigned int stage_id = 1; stage_id < COMPUTE_LATENCY; stage_id++) {
      result_pipeline[stage_id].write(result_pipeline[stage_id-1].read());
      valid_pipeline[RF_RD_LATENCY+stage_id].write(valid_pipeline[RF_RD_LATENCY+stage_id-1].read());
      dest_layer_pipeline[RF_RD_LATENCY+stage_id].write(dest_layer_pipeline[RF_RD_LATENCY+stage_id-1].read());
      dest_mvm_pipeline[RF_RD_LATENCY+stage_id].write(dest_mvm_pipeline[RF_RD_LATENCY+stage_id-1].read());
    }

    if (rx_interface.tvalid.read() && rx_interface.tready.read()) {
      sc_bv<AXIS_MAX_DATAW> tdata = rx_interface.tdata.read();

      if (rx_interface.tuser.read().range(15,13).to_uint() == 1) {
        uint32_t waddr = rx_interface.tuser.read().range(8,0).to_uint();
        mvm_inst inst;
        inst.from_bv(rx_interface.tdata.read());
        inst_memory[waddr] = inst;
        ififo_wen_signal.write(false);
        reduce_fifo_wen_signal.write(false);
        for (unsigned int dot_id = 0; dot_id < DOT_PRODUCTS; dot_id++) {
          matrix_mem_wen[dot_id].write(false);
        }
      } else if (rx_interface.tuser.read().range(15,13).to_uint() > 1) {
        // Read rx tdata into a vector
        for (unsigned int lane_id = 0; lane_id < LANES; lane_id++) {
          tdata_vec[lane_id] = tdata.range((lane_id + 1) * 32 - 1, lane_id * 32).to_int();
        }

        // Push the data vector into the right FIFO/memory
        if (rx_interface.tuser.read().range(15,13).to_uint() == 2) {         // Reduction FIFO
          reduce_fifo_wdata_signal.write(tdata_vec);
          reduce_fifo_wen_signal.write(true);
          ififo_wen_signal.write(false);
          for (unsigned int dot_id = 0; dot_id < DOT_PRODUCTS; dot_id++) {
            matrix_mem_wen[dot_id].write(false);
          }
          //std::cout << "Write to reduce FIFO" << std::endl;
          //std::cout << tdata_vec << std::endl;
        } else if (rx_interface.tuser.read().range(15,13).to_uint() == 3) {  // Input FIFO
          ififo_wdata_signal.write(tdata_vec);
          ififo_wen_signal.write(true);
          reduce_fifo_wen_signal.write(false);
          for (unsigned int dot_id = 0; dot_id < DOT_PRODUCTS; dot_id++) {
            matrix_mem_wen[dot_id].write(false);
          }
          //std::cout << "MVM got input data" << std::endl;
        } else if (rx_interface.tuser.read().range(15,13).to_uint() == 4) {  // Matrix memory
          uint32_t waddr = rx_interface.tuser.read().range(8,0).to_uint();
          uint32_t wen_id = rx_interface.tuser.read().range(12,9).to_uint();
          matrix_mem_waddr.write(waddr);
          matrix_mem_wdata.write(tdata_vec);
          for (unsigned int dot_id = 0; dot_id < DOT_PRODUCTS; dot_id++) {
            if (dot_id == wen_id) matrix_mem_wen[wen_id].write(true);
            else matrix_mem_wen[dot_id].write(false);
          }
          ififo_wen_signal.write(false);
          reduce_fifo_wen_signal.write(false);
        } else {
          ififo_wen_signal.write(false);
          reduce_fifo_wen_signal.write(false);
          for (unsigned int dot_id = 0; dot_id < DOT_PRODUCTS; dot_id++) {
            matrix_mem_wen[dot_id].write(false);
          }
        }
      } else {
        ififo_wen_signal.write(false);
        reduce_fifo_wen_signal.write(false);
        for (unsigned int dot_id = 0; dot_id < DOT_PRODUCTS; dot_id++) {
          matrix_mem_wen[dot_id].write(false);
        }
      }
    } else {
      ififo_wen_signal.write(false);
      reduce_fifo_wen_signal.write(false);
      for (unsigned int dot_id = 0; dot_id < DOT_PRODUCTS; dot_id++) {
        matrix_mem_wen[dot_id].write(false);
      }
    }
    wait();
  } 
}

void mvm::Assign() {
  if (rst.read()) {
    rx_interface.tready.write(false);
    tx_interface.tdata.write(0);
    tx_interface.tvalid.write(false);
    tx_interface.tstrb.write((2 << AXIS_STRBW) - 1);
    tx_interface.tkeep.write((2 << AXIS_KEEPW) - 1);
    tx_interface.tlast.write(0);
    tx_interface.tuser.write(0);
    ififo_ren_signal.write(false);
    reduce_fifo_ren_signal.write(false);
    ofifo_wen_signal.write(false);
    ofifo_ren_signal.write(false);
    dl_fifo_ren_signal.write(false);
    dm_fifo_ren_signal.write(false);
    matrix_mem_clk_en.write(false);
    matrix_mem_raddr.write(0);
    dot_op.write(false);
    dot_reduce_op.write(false);
  } else {
    if (rx_interface.tuser.read().range(15, 13).to_uint() == 1) {         // Inst memory
      rx_interface.tready.write(true);
    } else if (rx_interface.tuser.read().range(15, 13).to_uint() == 2) {  // Reduction FIFO
      rx_interface.tready.write(!reduce_fifo_almost_full_signal.read());
    } else if (rx_interface.tuser.read().range(15, 13).to_uint() == 3) {  // Input FIFO
      rx_interface.tready.write(!ififo_almost_full_signal.read());
    } else if (rx_interface.tuser.read().range(15, 13).to_uint() == 4) {  // Matrix memory
      rx_interface.tready.write(true);
    } else {
      rx_interface.tready.write(false);
    }

    matrix_mem_raddr.write(next_inst.read().raddr);
    next_inst.write(inst_memory[pc.read()]);

    if (!ififo_empty_signal && !reduce_fifo_empty_signal && !ofifo_almost_full_signal
      && next_inst.read().en && !next_inst.read().jump && next_inst.read().reduce) {
      ififo_ren_signal.write(next_inst.read().last);
      reduce_fifo_ren_signal.write(true);
      dot_reduce_op.write(true);
      dot_op.write(false);
    } else if (!ififo_empty_signal && !ofifo_almost_full_signal
      && next_inst.read().en && !next_inst.read().jump && !next_inst.read().reduce) {
      ififo_ren_signal.write(next_inst.read().last);
      reduce_fifo_ren_signal.write(false);
      dot_op.write(true);
      dot_reduce_op.write(false);
    } else {
      ififo_ren_signal.write(false);
      reduce_fifo_ren_signal.write(false);
      dot_op.write(false);
      dot_reduce_op.write(false);
    }

    ofifo_wen_signal.write(valid_pipeline[COMPUTE_LATENCY+RF_RD_LATENCY-1].read());
    ofifo_wdata_signal.write(result_pipeline[COMPUTE_LATENCY-1].read());

    data_vector<sc_int<5>> dest_layer(1);
    dest_layer[0] = dest_layer_pipeline[COMPUTE_LATENCY+RF_RD_LATENCY-1].read();
    dl_fifo_wen_signal.write(valid_pipeline[COMPUTE_LATENCY+RF_RD_LATENCY-1].read());
    dl_fifo_wdata_signal.write(dest_layer);

    data_vector<sc_uint<5>> dest_mvm(1);
    dest_mvm[0] = dest_mvm_pipeline[COMPUTE_LATENCY+RF_RD_LATENCY-1].read();
    dm_fifo_wen_signal.write(valid_pipeline[COMPUTE_LATENCY+RF_RD_LATENCY-1].read());
    dm_fifo_wdata_signal.write(dest_mvm);

    data_vector<sc_int<32>> tx_tdata = ofifo_rdata_signal.read();
    data_vector<sc_int<5>> dest_layer_vec; 
    dest_layer_vec = dl_fifo_rdata_signal.read();
    int dest_layer_int = 0;
    data_vector<sc_uint<5>> dest_mvm_vec;
    dest_mvm_vec = dm_fifo_rdata_signal.read();
    unsigned int dest_mvm_int = 0;
    if (dest_layer_vec.size() > 0) {
      dest_layer_int = dest_layer_vec[0].to_int();
      dest_mvm_int = dest_mvm_vec[0].to_uint();
    }
    std::string dest_name;
    unsigned int dest_id;
    if (dest_layer_int == 0) {
      dest_name = "output_collector.data_collect";
    } else {
      dest_name = "layer" + std::to_string(dest_layer_int-1) + "_mvm" + std::to_string(dest_mvm_int) + ".rx_interface";
    }
    dest_id = radsim_design.GetPortDestinationID(dest_name);

    unsigned int dest_interface;
    // If destination is the same layer, send to reduce FIFO
    if ((unsigned int) dest_layer_int-1 == layer_id) {
      dest_interface = 2 << 13;
      //if (tx_tdata.size() > 0 && !ofifo_empty_signal)
        //std::cout << this->name() << " sending to interface 2 -- " << dest_layer_int << std::endl;
    }
    // If destination is a different layer, send to the input FIFO
    else {
      dest_interface = 3 << 13;
      //if (tx_tdata.size() > 0 && !ofifo_empty_signal)
        //std::cout << this->name() << " sending to interface 3 -- " << dest_layer_int << std::endl;
    }

    if (tx_tdata.size() > 0 && !ofifo_empty_signal) {
      sc_bv<AXIS_MAX_DATAW> tx_tdata_bv;
      for (unsigned int lane_id = 0; lane_id < LANES; lane_id++) {
        tx_tdata_bv.range((lane_id + 1) * 32 - 1, lane_id * 32) = tx_tdata[lane_id];
      }
      tx_interface.tdata.write(tx_tdata_bv);
      tx_interface.tvalid.write(!ofifo_empty_signal);
      tx_interface.tuser.write(dest_interface);
      tx_interface.tdest.write(dest_id);
      /*if (dest_interface == 2 << 13 && !ofifo_empty_signal) {
        std::cout << "Sending to reduce FIFO" << std::endl;
        std::cout << tx_tdata << std::endl;
      }*/
      //std::cout << "MVM (" << layer_id << "," << mvm_id << ") pushed data into the NoC with dest " << dest_id << "!" << std::endl;
    } else {
      tx_interface.tvalid.write(false);
    }

    ofifo_ren_signal.write(tx_interface.tvalid.read() && tx_interface.tready.read());
    dl_fifo_ren_signal.write(tx_interface.tvalid.read() && tx_interface.tready.read());
    dm_fifo_ren_signal.write(tx_interface.tvalid.read() && tx_interface.tready.read());
  }
}

void mvm::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_slave_ports = 0;
  _num_noc_master_ports = 0;

  port_name = module_name + ".tx_interface";
  RegisterMasterPort(port_name, &tx_interface, 512, 0);

  port_name = module_name + ".rx_interface";
  RegisterSlavePort(port_name, &rx_interface, 512, 0);
}
