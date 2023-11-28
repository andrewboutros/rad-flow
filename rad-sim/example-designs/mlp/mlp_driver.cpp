#include "mlp_driver.hpp"

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
  if(!design_config_file) {
    std::cerr << "Cannot read MLP design configuration file!" << std::endl;
    exit(1);
  }
  std::string line;
  std::getline(design_config_file, line);
  std::stringstream line_stream(line);
  unsigned int tmp;
  line_stream >> num_layers;
  num_mvms.resize(num_layers);
  for (unsigned int layer_id = 0; layer_id < num_layers; layer_id++) {
    line_stream >> tmp;
    num_mvms[layer_id] = tmp;
  }

  // Intialize input/output interface vectors
  init_vector<sc_in<bool>>::init_sc_vector(dispatcher_fifo_rdy, num_mvms[0]);
  init_vector<sc_out<bool>>::init_sc_vector(dispatcher_fifo_wen, num_mvms[0]);
  init_vector<sc_out<data_vector<sc_int<32>>>>::init_sc_vector(dispatcher_fifo_wdata, num_mvms[0]);

  // Parse test inputs
  test_inputs.resize(num_mvms[0]);
  std::string filename;
  for (unsigned int dispatcher_id = 0; dispatcher_id < num_mvms[0]; dispatcher_id++) {
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
  for (unsigned int dispatcher_id = 0; dispatcher_id < num_mvms[0]; dispatcher_id++)
    dispatcher_fifo_wen[dispatcher_id].write(false);
  wait();
  rst.write(false);
  start_cycle = GetSimulationCycle(1.0);
  wait();

  std::vector<unsigned int> written_inputs(num_mvms[0], 0);
  bool still_have_inputs_to_feed = true;
  while (still_have_inputs_to_feed) {
    for (unsigned int dispatcher_id = 0; dispatcher_id < num_mvms[0]; dispatcher_id++) {
      if (dispatcher_fifo_rdy[dispatcher_id].read() 
        && written_inputs[dispatcher_id] < test_inputs[dispatcher_id].size()) {
        data_vector<sc_int<32>> tmp(test_inputs[dispatcher_id][written_inputs[dispatcher_id]]);
        dispatcher_fifo_wen[dispatcher_id].write(true);
        dispatcher_fifo_wdata[dispatcher_id].write(tmp);
        written_inputs[dispatcher_id]++;
      } else {
        dispatcher_fifo_wen[dispatcher_id].write(false);
      }
    }
    wait();
  }
  std::cout << "Finished writing all test inputs!" << std::endl;
  for (unsigned int dispatcher_id = 0; dispatcher_id < num_mvms[0]; dispatcher_id++)
    dispatcher_fifo_wen[dispatcher_id].write(false);
  wait();
}

void mlp_driver::sink() {
  unsigned int read_outputs = 0;
  bool mistake = false;

  data_vector<sc_int<32>> output_vec;
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
  }
  else std::cout << "SUCCESS - All outputs are matching!" << std::endl;

  end_cycle = GetSimulationCycle(1.0);
  std::cout << "Simulation Cycles = " << end_cycle - start_cycle << std::endl;
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