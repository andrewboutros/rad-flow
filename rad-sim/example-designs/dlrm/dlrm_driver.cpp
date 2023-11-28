#include <dlrm_driver.hpp>

bool ParseInputs(std::vector<data_vector<uint64_t>> &lookup_indecies,
                 std::vector<data_vector<unsigned int>> &target_channels,
                 std::vector<data_vector<uint64_t>> &base_addresses,
                 std::string &io_filename) {
  std::ifstream io_file(io_filename);
  if (!io_file)
    return false;

  uint64_t num_indecies_per_input, index;
  std::string line;

  // Get number of indecies per input
  std::getline(io_file, line);
  std::stringstream header_stream(line);
  header_stream >> num_indecies_per_input;

  unsigned int line_num = 0;
  while (std::getline(io_file, line)) {
    std::stringstream line_stream(line);
    if (line_num % 3 == 0) {
      data_vector<uint64_t> dvector(num_indecies_per_input);
      for (unsigned int i = 0; i < num_indecies_per_input; i++) {
        line_stream >> index;
        dvector[i] = index;
      }
      lookup_indecies.push_back(dvector);
    } else if (line_num % 3 == 1) {
      data_vector<unsigned int> dvector(num_indecies_per_input);
      for (unsigned int i = 0; i < num_indecies_per_input; i++) {
        line_stream >> index;
        dvector[i] = index;
      }
      target_channels.push_back(dvector);
    } else {
      data_vector<uint64_t> dvector(num_indecies_per_input);
      for (unsigned int i = 0; i < num_indecies_per_input; i++) {
        line_stream >> index;
        dvector[i] = index;
      }
      base_addresses.push_back(dvector);
    }
    line_num++;
  }
  return true;
}

bool ParseOutputs(std::vector<std::vector<int16_t>> &fi_outputs,
                  std::string &io_filename, unsigned int &num_outputs) {
  std::ifstream io_file(io_filename);
  if (!io_file)
    return false;

  int16_t element;
  std::string line;

  std::getline(io_file, line);
  std::stringstream line_stream(line);
  line_stream >> num_outputs;

  while (std::getline(io_file, line)) {
    std::stringstream line_stream(line);
    std::vector<int16_t> tmp;
    while (line_stream.rdbuf()->in_avail() != 0) {
      line_stream >> element;
      tmp.push_back(element);
    }
    fi_outputs.push_back(tmp);
  }
  return true;
}

dlrm_driver::dlrm_driver(const sc_module_name &name) : sc_module(name) {

  // Parse design configuration (number of layers & number of MVM per layer)
  std::string design_root_dir =
      radsim_config.GetStringKnob("radsim_user_design_root_dir");

  std::string inputs_filename =
      design_root_dir + "/compiler/embedding_indecies.in";
  ParseInputs(_lookup_indecies, _target_channels, _base_addresses,
              inputs_filename);
  std::cout << "Finished parsing inputs!" << std::endl;

  std::string feature_interaction_outputs_filename =
      design_root_dir + "/compiler/feature_interaction.out";
  ParseOutputs(_feature_interaction_outputs,
               feature_interaction_outputs_filename,
               _num_feature_interaction_outputs);

  std::string mlp_outputs_filename = design_root_dir + "/compiler/mlp.out";
  ParseOutputs(_mlp_outputs, mlp_outputs_filename, _num_mlp_outputs);

  SC_METHOD(assign);
  sensitive << collector_fifo_rdy;
  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

dlrm_driver::~dlrm_driver() {}

void dlrm_driver::assign() { collector_fifo_ren.write(collector_fifo_rdy); }

void dlrm_driver::source() {
  // Reset
  rst.write(true);
  lookup_indecies_valid.write(false);
  wait();
  rst.write(false);
  wait();

  unsigned int idx = 0;
  _start_cycle =
      GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
  while (idx < _lookup_indecies.size()) {
    lookup_indecies_data.write(_lookup_indecies[idx]);
    lookup_indecies_target_channels.write(_target_channels[idx]);
    lookup_indecies_base_addresses.write(_base_addresses[idx]);
    lookup_indecies_valid.write(true);

    wait();

    if (lookup_indecies_valid.read() && lookup_indecies_ready.read()) {
      idx++;
    }
  }
  lookup_indecies_valid.write(false);
  std::cout << this->name()
            << ": Finished sending all inputs to embedding lookup module!"
            << std::endl;
  wait();
}

void print_progress_bar(unsigned int outputs_count, unsigned int total) {
  unsigned int loading_bar_width = 50;
  std::cout << "[";
  float progress = 1.0 * outputs_count / total;
  unsigned int pos = loading_bar_width * progress;
  for (unsigned int i = 0; i < loading_bar_width; ++i) {
    if (i < pos)
      std::cout << "=";
    else if (i == pos)
      std::cout << ">";
    else
      std::cout << " ";
  }
  if (outputs_count == total) {
    std::cout << "] " << int(progress * 100.0) << " %\n";
  } else {
    std::cout << "] " << int(progress * 100.0) << " %\r";
  }
  std::cout.flush();
}

void dlrm_driver::sink() {
  std::ofstream mismatching_outputs_file("mismatching.log");

  unsigned int outputs_count = 0;
  data_vector<int16_t> dut_output;
  bool all_outputs_matching = true;
  while (outputs_count < _num_mlp_outputs) {
    dut_output = collector_fifo_rdata.read();
    if (collector_fifo_rdy.read() && dut_output.size() > 0) {
      bool matching = true;
      for (unsigned int e = 0; e < dut_output.size(); e++) {
        matching = (dut_output[e] == _mlp_outputs[outputs_count][e]);
      }
      if (!matching) {
        std::cout << "Output " << outputs_count << " does not match!\n";
        std::cout << "TRUE: [ ";
        for (unsigned int e = 0; e < _mlp_outputs[outputs_count].size(); e++) {
          std::cout << _mlp_outputs[outputs_count][e] << " ";
        }
        std::cout << "]\n";
        std::cout << "DUT : [ ";
        for (unsigned int e = 0; e < dut_output.size(); e++) {
          std::cout << dut_output[e] << " ";
        }
        std::cout << "]\n";
        std::cout << "-------------------------------\n";
      }
      outputs_count++;
      all_outputs_matching &= matching;

      print_progress_bar(outputs_count, _num_mlp_outputs);
    }
    wait();
  }
  std::cout << "Got " << outputs_count << " output(s)!\n";
  mismatching_outputs_file.flush();
  mismatching_outputs_file.close();

  if (all_outputs_matching) {
    std::cout << "Simulation PASSED! All outputs matching!" << std::endl;
  } else {
    std::cout << "Simulation FAILED! Some outputs are NOT matching!" << std::endl;
    radsim_design.ReportDesignFailure();
  }
  _end_cycle =
      GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
  std::cout << "Simulated " << (_end_cycle - _start_cycle) << " cycle(s)"
            << std::endl;

  for (unsigned int i = 0; i < 10; i++) {
    wait();
  }
  sc_stop();
}