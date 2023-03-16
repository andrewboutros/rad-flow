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

bool ParseFeatureInteractionOutputs(
    std::vector<std::vector<int16_t>> &fi_outputs, std::string &io_filename,
    unsigned int &num_outputs) {
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
  ParseFeatureInteractionOutputs(_feature_interaction_outputs,
                                 feature_interaction_outputs_filename,
                                 _num_outputs);

  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

dlrm_driver::~dlrm_driver() {}

void dlrm_driver::source() {
  // Reset
  rst.write(true);
  lookup_indecies_valid.write(false);
  feature_interaction_ready.write(true);
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

void dlrm_driver::sink() {
  unsigned int outputs_count = 0;
  data_vector<int16_t> dut_output;
  bool all_outputs_matching = true;
  while (outputs_count < _num_outputs) {
    if (feature_interaction_ready.read() && feature_interaction_valid.read()) {
      bool matching = true;
      dut_output = feature_interaction_odata.read();
      for (unsigned int element_id = 0; element_id < dut_output.size();
           element_id++) {
        matching = (dut_output[element_id] ==
                    _feature_interaction_outputs[outputs_count][element_id]);
      }
      if (!matching) {
        std::cout << "Output " << outputs_count << " does not match!"
                  << std::endl;
        std::cout << "TRUE: [ ";
        for (unsigned int element_id = 0;
             element_id < _feature_interaction_outputs[outputs_count].size();
             element_id++) {
          std::cout << _feature_interaction_outputs[outputs_count][element_id]
                    << " ";
        }
        std::cout << std::endl;
        std::cout << "DUT : [ ";
        for (unsigned int element_id = 0; element_id < dut_output.size();
             element_id++) {
          std::cout << dut_output[element_id] << " ";
        }
        std::cout << std::endl;
        std::cout << "-------------------------------" << std::endl;
      }
      outputs_count++;
      all_outputs_matching &= matching;
    }
    wait();
  }
  if (all_outputs_matching) {
    std::cout << "Simulation PASSED! All outputs matching!" << std::endl;
  } else {
    std::cout << "Simulation FAILED! Some outputs are NOT matching!"
              << std::endl;
  }
  _end_cycle =
      GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
  std::cout << "Simulated " << (_end_cycle - _start_cycle) << " cycle(s)"
            << std::endl;
  sc_stop();
}