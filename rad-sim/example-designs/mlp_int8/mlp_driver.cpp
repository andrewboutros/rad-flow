#include "mlp_driver.hpp"

bool ParseWeights(std::vector<std::vector<int>>& weights, 
  std::vector<unsigned int>& rf_ids, std::vector<unsigned int>& rf_addrs,
  std::vector<unsigned int>& layer_ids, std::vector<unsigned int>& mvm_ids,
  unsigned int num_layers, std::vector<unsigned int>& num_mvms) { 
  
  std::string design_root_dir = radsim_config.GetStringKnob("radsim_user_design_root_dir");
  for (unsigned int l = 0; l < num_layers; l++) {
    for (unsigned int m = 0; m < num_mvms[l]; m++) {
      for (unsigned int d = 0; d < DPES; d++) {

        std::string filename = design_root_dir + "/compiler/weight_mifs/layer" +
          std::to_string(l) + "_mvm" + std::to_string(m) + "_dot" + std::to_string(d) + ".mif";
        
        std::ifstream io_file(filename);
        if(!io_file)
          return false;

        std::string line;
        unsigned int addr = 0;
        while(std::getline(io_file, line)) {
          std::stringstream line_stream(line);
          std::vector<int> values(LANES, 0);
          int value;
          for (unsigned int i = 0; i < LANES; i++) {
            line_stream >> value;
            values[i] = value;
          }
          weights.push_back(values);
          rf_addrs.push_back(addr);
          rf_ids.push_back(d);
          layer_ids.push_back(l);
          mvm_ids.push_back(m);
          addr++;
        }
      }
    }
  }
  return true;
}

bool ParseInstructions(std::vector<mvm_inst> &insts, 
  std::vector<unsigned int>& layer_ids,
  std::vector<unsigned int>& mvm_ids, 
  unsigned int num_layers, std::vector<unsigned int>& num_mvms) {
  std::string design_root_dir = radsim_config.GetStringKnob("radsim_user_design_root_dir");
  
  for (unsigned int l = 0; l < num_layers; l++) {
    for (unsigned int m = 0; m < num_mvms[l]; m++) {
      std::string inst_filename = design_root_dir + "/compiler/inst_mifs/layer" +
              std::to_string(l) + "_mvm" + std::to_string(m) + ".mif";
            
      std::ifstream inst_file(inst_filename);
      if (!inst_file)
        return false;

      std::string line;
      while (std::getline(inst_file, line)) {
        std::stringstream line_stream(line);
        mvm_inst inst;
        uint32_t value;

        line_stream >> value;
        inst.release_op = value;
        line_stream >> value;
        inst.release_dest = value;
        line_stream >> value;
        inst.rf_raddr = value;
        line_stream >> value;
        inst.accum_raddr = value;
        line_stream >> value;
        inst.last = value;
        line_stream >> value;
        inst.release = value;
        line_stream >> value;
        inst.accum_en = value;
        line_stream >> value;
        inst.reduce = value;

        insts.push_back(inst);
        layer_ids.push_back(l);
        mvm_ids.push_back(m);
      }
    }
  }
  return true;
}

bool ParseIO(std::vector<std::vector<int>>& data_vec, std::string& io_filename) {  
  std::ifstream io_file(io_filename);
  if(!io_file)
    return false;

  std::string line;
  while(std::getline(io_file, line)) {
    std::stringstream line_stream(line);
    std::vector<int> values(LANES, 0);
    int value;
    for (unsigned int i = 0; i < LANES; i++) {
      line_stream >> value;
      values[i] = value;
    }
    data_vec.push_back(values);
  }
  return true;
}

mlp_driver::mlp_driver(const sc_module_name& name) : sc_module(name) {
  start_cycle = 0;
  end_cycle = 0;

  // Parse design configuration (number of layers & number of MVM per layer)
  std::string design_root_dir = radsim_config.GetStringKnob("radsim_user_design_root_dir");
  std::string design_config_filename = design_root_dir + "/compiler/layer_mvm_config";
  std::ifstream design_config_file(design_config_filename);
  if (!design_config_file) {
    std::cerr << "Cannot read MLP design configuration file!" << std::endl;
    exit(1);
  }
  std::string line;
  std::getline(design_config_file, line);
  std::stringstream line_stream(line);
  std::string num_mvms_layer, num_mvms_rtl_layer;
  std::string layer_mvms;
  line_stream >> num_layers;
  num_mvms.resize(num_layers);
  num_mvms_rtl.resize(num_layers);
  num_mvms_total.resize(num_layers);
  for (unsigned int layer_id = 0; layer_id < num_layers; layer_id++) {
    line_stream >> layer_mvms;
    std::stringstream layer_mvms_stream(layer_mvms);
    std::getline(layer_mvms_stream, num_mvms_layer, ',');
    std::getline(layer_mvms_stream, num_mvms_rtl_layer, ',');
    num_mvms[layer_id] = std::stoi(num_mvms_layer);
    num_mvms_rtl[layer_id] = std::stoi(num_mvms_rtl_layer);
    num_mvms_total[layer_id] = num_mvms[layer_id] + num_mvms_rtl[layer_id];
  }

  // Intialize input/output interface vectors
  init_vector<sc_in<bool>>::init_sc_vector(dispatcher_fifo_rdy, num_mvms_total[0]);
  init_vector<sc_out<bool>>::init_sc_vector(dispatcher_fifo_wen, num_mvms_total[0]);
  init_vector<sc_out<data_vector<sc_int<IPRECISION>>>>::init_sc_vector(dispatcher_fifo_wdata, num_mvms_total[0]);

  // Parse weights
  ParseWeights(weight_data, weight_rf_id, weight_rf_addr, weight_layer_id, 
    weight_mvm_id, num_layers, num_mvms_total);
  std::cout << "# Weight vectors = " << weight_data.size() << std::endl;

  // Parse instructions
  ParseInstructions(inst_data, inst_layer_id, inst_mvm_id, num_layers, num_mvms_total); 
  std::cout << "# Instructions = " << inst_data.size() << std::endl;

  // Parse test inputs
  test_inputs.resize(num_mvms_total[0]);
  std::string filename;
  for (unsigned int dispatcher_id = 0; dispatcher_id < num_mvms_total[0]; dispatcher_id++) {
    filename = design_root_dir + "/compiler/input_mifs/inputs_mvm" + std::to_string(dispatcher_id) + ".mif";
    if (!ParseIO(test_inputs[dispatcher_id], filename)) {
      std::cout << "Failed to parse test inputs file: " << filename << std::endl;
      exit(1);
    }
  }
  std::cout << "# Inputs per dispatcher = " << test_inputs[0].size() << std::endl;

  // Parse golden outputs
  filename = design_root_dir + "/compiler/golden_outputs.mif";
  if (!ParseIO(golden_outputs, filename)) {
    std::cout << "Failed to parse golden outputs file: " << filename << std::endl;
    exit(1);
  }
  std::cout << "# Outputs = " << golden_outputs.size() << std::endl;

  SC_METHOD(assign);
  sensitive << collector_fifo_rdy;
  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

mlp_driver::~mlp_driver() { }

void mlp_driver::source() {
  // Reset
  rst.write(true);
  weight_loader_weight_fifo_wen.write(false);
  weight_loader_rf_id_fifo_wen.write(false);
  weight_loader_rf_addr_fifo_wen.write(false);
  weight_loader_layer_id_fifo_wen.write(false);
  weight_loader_mvm_id_fifo_wen.write(false);
  inst_loader_inst_fifo_wen.write(false);
  inst_loader_layer_id_fifo_wen.write(false);
  inst_loader_mvm_id_fifo_wen.write(false);
  for (unsigned int dispatcher_id = 0; dispatcher_id < num_mvms_total[0]; dispatcher_id++)
    dispatcher_fifo_wen[dispatcher_id].write(false);
  wait();
  rst.write(false);
  wait();

  // Write weights
  unsigned int written_weights = 0;
  while (written_weights < weight_data.size()) {
    if (weight_loader_weight_fifo_rdy.read()) {
      data_vector<sc_int<IPRECISION>> tmp(weight_data[written_weights]);
      weight_loader_weight_fifo_wdata.write(tmp);
      weight_loader_weight_fifo_wen.write(true);
      weight_loader_rf_id_fifo_wdata.write(weight_rf_id[written_weights]);
      weight_loader_rf_id_fifo_wen.write(true);
      weight_loader_rf_addr_fifo_wdata.write(weight_rf_addr[written_weights]);
      weight_loader_rf_addr_fifo_wen.write(true);
      weight_loader_layer_id_fifo_wdata.write(weight_layer_id[written_weights]);
      weight_loader_layer_id_fifo_wen.write(true);
      weight_loader_mvm_id_fifo_wdata.write(weight_mvm_id[written_weights]);
      weight_loader_mvm_id_fifo_wen.write(true);
      written_weights++;
    } else {
      weight_loader_weight_fifo_wen.write(false);
      weight_loader_rf_id_fifo_wen.write(false);
      weight_loader_rf_addr_fifo_wen.write(false);
      weight_loader_layer_id_fifo_wen.write(false);
      weight_loader_mvm_id_fifo_wen.write(false);
    }
    wait();
  }
  std::cout << "Finished writing all weights!" << std::endl;
  weight_loader_weight_fifo_wen.write(false);
  weight_loader_rf_id_fifo_wen.write(false);
  weight_loader_rf_addr_fifo_wen.write(false);
  weight_loader_layer_id_fifo_wen.write(false);
  weight_loader_mvm_id_fifo_wen.write(false);
  wait();

  // Write instructions
  unsigned int written_insts = 0;
  while (written_insts < inst_data.size()) {
    if (inst_loader_inst_fifo_rdy.read()) {
      inst_loader_inst_fifo_wdata.write(inst_data[written_insts]);
      inst_loader_inst_fifo_wen.write(true);
      inst_loader_layer_id_fifo_wdata.write(inst_layer_id[written_insts]);
      inst_loader_layer_id_fifo_wen.write(true);
      inst_loader_mvm_id_fifo_wdata.write(inst_mvm_id[written_insts]);
      inst_loader_mvm_id_fifo_wen.write(true);
      written_insts++;
    } else {
      inst_loader_inst_fifo_wen.write(false);
      inst_loader_layer_id_fifo_wen.write(false);
      inst_loader_mvm_id_fifo_wen.write(false);
    }
    wait();
  }
  std::cout << "Finished writing all instructions!" << std::endl;
  inst_loader_inst_fifo_wen.write(false);
  inst_loader_layer_id_fifo_wen.write(false);
  inst_loader_mvm_id_fifo_wen.write(false);
  wait();

  unsigned int wait_cycles = 10000;
  for (unsigned int i = 0; i < wait_cycles; i++) {
    wait();
  }

  start_cycle = GetSimulationCycle(1.0);
  start_time = std::chrono::steady_clock::now();
  wait();

  std::vector<unsigned int> written_inputs(num_mvms_total[0], 0);
  bool still_have_inputs_to_feed = true;
  while (still_have_inputs_to_feed) {
    for (unsigned int dispatcher_id = 0; dispatcher_id < num_mvms_total[0]; dispatcher_id++) {
      if (dispatcher_fifo_rdy[dispatcher_id].read() 
        && written_inputs[dispatcher_id] < test_inputs[dispatcher_id].size()) {
        data_vector<sc_int<IPRECISION>> tmp(test_inputs[dispatcher_id][written_inputs[dispatcher_id]]);
        dispatcher_fifo_wen[dispatcher_id].write(true);
        dispatcher_fifo_wdata[dispatcher_id].write(tmp);
        written_inputs[dispatcher_id]++;
      } else {
        dispatcher_fifo_wen[dispatcher_id].write(false);
      }
    }
    still_have_inputs_to_feed = false;
    for (unsigned int i = 0; i < num_mvms_total[0]; i++)
      still_have_inputs_to_feed = still_have_inputs_to_feed || 
        (written_inputs[i] < test_inputs[i].size());
    wait();
  }
  std::cout << "Finished writing all test inputs!" << std::endl;
  for (unsigned int dispatcher_id = 0; dispatcher_id < num_mvms_total[0]; dispatcher_id++)
    dispatcher_fifo_wen[dispatcher_id].write(false);
  wait();
}

void mlp_driver::sink() {
  unsigned int read_outputs = 0;
  bool mistake = true;

  data_vector<sc_int<IPRECISION>> output_vec;
  while (read_outputs < golden_outputs.size()) {
    output_vec = collector_fifo_rdata.read();
    if (collector_fifo_rdy.read() && output_vec.size() > 0) {
      for (unsigned int i = 0; i < output_vec.size(); i++) {
        if (output_vec[i] != golden_outputs[read_outputs][i]) {
          mistake = true;
        }
        //std::cout << output_vec[i] << " -- " << golden_outputs[read_outputs][i] << std::endl;
      }
      //std::cout << "--------------------------" << std::endl;
      read_outputs++;
    }
    wait();
  }
  if (mistake) {
    std::cout << "FAILURE - Some outputs NOT matching!" << std::endl;
    radsim_design.ReportDesignFailure();
  } else {
    std::cout << "SUCCESS - All outputs are matching!" << std::endl;
  }

  end_cycle = GetSimulationCycle(1.0);
  end_time = std::chrono::steady_clock::now();
  std::cout << "Simulation Cycles = " << end_cycle - start_cycle << std::endl;
  std::cout << "Simulation Time = " << std::chrono::duration_cast<std::chrono::milliseconds> (end_time - start_time).count() << " ms" << std::endl;
  NoCTransactionTelemetry::DumpStatsToFile("stats.csv");
  NoCFlitTelemetry::DumpNoCFlitTracesToFile("flit_traces.csv");

  std::vector<double> aggregate_bandwidths = NoCTransactionTelemetry::DumpTrafficFlows("traffic_flows", 
    end_cycle - start_cycle, radsim_design.GetNodeModuleNames());
  std::cout << "Aggregate NoC BW = " << aggregate_bandwidths[0] / 1000000000 << " Gbps" << std::endl;

  sc_stop();
}

void mlp_driver::assign() {
  collector_fifo_ren.write(collector_fifo_rdy);
}