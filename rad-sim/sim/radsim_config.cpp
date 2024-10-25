#include "radsim_config.hpp"

RADSimConfig::RADSimConfig() {}
RADSimConfig::~RADSimConfig() {}

// template<class T> 
// void insert_with_resize(std::vector<> ) {

// }

void RADSimConfig::ResizeAll(int num_rads) {
  _int_knobs_per_rad.resize(num_rads);
  _double_knobs_per_rad.resize(num_rads);
  _string_knobs_per_rad.resize(num_rads);
  _int_vector_knobs_per_rad.resize(num_rads);
  _double_vector_knobs_per_rad.resize(num_rads);
  _string_vector_knobs_per_rad.resize(num_rads);
}


// Adds a new integer configuration knob
void RADSimConfig::AddIntKnobShared(const std::string &key, int val) {
  _int_knobs_shared[key] = val;
}

// Adds a new double configuration knob
void RADSimConfig::AddDoubleKnobShared(const std::string &key, double val) {
  _double_knobs_shared[key] = val;
}

// Adds a new string configuration knob
void RADSimConfig::AddStringKnobShared(const std::string &key, std::string &val) {
  _string_knobs_shared[key] = val;
}

// Adds a new integer vector configuration knob
void RADSimConfig::AddIntVectorKnobShared(const std::string &key,
                                    std::vector<int> &val) {
  _int_vector_knobs_shared[key] = val;
}

// Adds a new double vector configuration knob
void RADSimConfig::AddDoubleVectorKnobShared(const std::string &key,
                                       std::vector<double> &val) {
  _double_vector_knobs_shared[key] = val;
}

// Adds a new string vector configuration knob
void RADSimConfig::AddStringVectorKnobShared(const std::string &key,
                                       std::vector<std::string> &val) {
  _string_vector_knobs_shared[key] = val;
}

//RAD-specific functions
// Adds a new integer configuration knob
void RADSimConfig::AddIntKnobPerRad(const std::string &key, int val, unsigned int rad_id) {
  _int_knobs_per_rad[rad_id][key] = val;
}

// Adds a new double configuration knob
void RADSimConfig::AddDoubleKnobPerRad(const std::string &key, double val, unsigned int rad_id) {
  _double_knobs_per_rad[rad_id][key] = val;
}

// Adds a new string configuration knob
void RADSimConfig::AddStringKnobPerRad(const std::string &key, std::string &val, unsigned int rad_id) {
  _string_knobs_per_rad[rad_id][key] = val;
}

// Adds a new integer vector configuration knob
void RADSimConfig::AddIntVectorKnobPerRad(const std::string &key,
                                    std::vector<int> &val, unsigned int rad_id) {
  _int_vector_knobs_per_rad[rad_id][key] = val;
}

// Adds a new double vector configuration knob
void RADSimConfig::AddDoubleVectorKnobPerRad(const std::string &key,
                                       std::vector<double> &val, unsigned int rad_id) {
  _double_vector_knobs_per_rad[rad_id][key] = val;
}

// Adds a new string vector configuration knob
void RADSimConfig::AddStringVectorKnobPerRad(const std::string &key,
                                       std::vector<std::string> &val, unsigned int rad_id) {
  _string_vector_knobs_per_rad[rad_id][key] = val;
}

// Gets the value of an integer configuration knob
int RADSimConfig::GetIntKnobShared(const std::string &key) {
  if (_int_knobs_shared.find(key) == _int_knobs_shared.end()) {
    std::cerr << "GetIntKnobShared: Cannot find configuration parameter \"" << key
              << "\"" << std::endl;
    exit(1);
  }
  return _int_knobs_shared[key];
}

// Gets the value of a double configuration knob
double RADSimConfig::GetDoubleKnobShared(const std::string &key) {
  if (_double_knobs_shared.find(key) == _double_knobs_shared.end()) {
    std::cerr << "GetDoubleKnobShared: Cannot find configuration parameter \"" << key
              << "\"" << std::endl;
    exit(1);
  }
  return _double_knobs_shared[key];
}

// Gets the value of a string configuration knob
std::string RADSimConfig::GetStringKnobShared(const std::string &key) {
  if (_string_knobs_shared.find(key) == _string_knobs_shared.end()) {
    std::cerr << "GetStringKnobShared: Cannot find configuration parameter \"" << key
              << "\"" << std::endl;
    exit(1);
  }
  return _string_knobs_shared[key];
}

// Gets the value of an integer vector configuration knob
int RADSimConfig::GetIntVectorKnobShared(const std::string &key, unsigned int idx) {
  if (_int_vector_knobs_shared.find(key) == _int_vector_knobs_shared.end()) {
    std::cerr << "GetIntVectorKnobShared: Cannot find configuration parameter \""
              << key << "\"" << std::endl;
    exit(1);
  }
  return _int_vector_knobs_shared[key][idx];
}

// Gets the value of a double vector configuration knob
double RADSimConfig::GetDoubleVectorKnobShared(const std::string &key,
                                         unsigned int idx) {
  if (_double_vector_knobs_shared.find(key) == _double_vector_knobs_shared.end()) {
    std::cerr << "GetDoubleVectorKnobShared: Cannot find configuration parameter \""
              << key << "\"" << std::endl;
    exit(1);
  }
  return _double_vector_knobs_shared[key][idx];
}

// Gets the value of a string vector configuration knob
std::string RADSimConfig::GetStringVectorKnobShared(const std::string &key,
                                              unsigned int idx) {
  if (_string_vector_knobs_shared.find(key) == _string_vector_knobs_shared.end()) {
    std::cerr << "GetStringVectorKnobShared: Cannot find configuration parameter \""
              << key << "\"" << std::endl;
    exit(1);
  }
  return _string_vector_knobs_shared[key][idx];
}

// Gets the value of an integer vector configuration knob
std::vector<int> &RADSimConfig::GetIntVectorKnobShared(const std::string &key) {
  if (_int_vector_knobs_shared.find(key) == _int_vector_knobs_shared.end()) {
    std::cerr << "GetIntVectorKnobShared: Cannot find configuration parameter \""
              << key << "\"" << std::endl;
    exit(1);
  }
  return _int_vector_knobs_shared[key];
}

// Gets the value of a double vector configuration knob
std::vector<double> &RADSimConfig::GetDoubleVectorKnobShared(const std::string &key) {
  if (_double_vector_knobs_shared.find(key) == _double_vector_knobs_shared.end()) {
    std::cerr << "GetDoubleVectorKnobShared: Cannot find configuration parameter \""
              << key << "\"" << std::endl;
    exit(1);
  }
  return _double_vector_knobs_shared[key];
}

// Gets the value of a string vector configuration knob
std::vector<std::string> &
RADSimConfig::GetStringVectorKnobShared(const std::string &key) {
  if (_string_vector_knobs_shared.find(key) == _string_vector_knobs_shared.end()) {
    std::cerr << "GetStringVectorKnobShared: Cannot find configuration parameter \""
              << key << "\"" << std::endl;
    exit(1);
  }
  return _string_vector_knobs_shared[key];
}

//Retrieve values of RAD-specific knobs
// Gets the value of an integer configuration knob
int RADSimConfig::GetIntKnobPerRad(const std::string &key, unsigned int rad_id) {
  if (_int_knobs_per_rad[rad_id].find(key) == _int_knobs_per_rad[rad_id].end()) {
    std::cerr << "GetIntKnobPerRAD: Cannot find configuration parameter \"" << key
              << "\"" << std::endl;
    exit(1);
  }
  return _int_knobs_per_rad[rad_id][key];
}

// Gets the value of a double configuration knob
double RADSimConfig::GetDoubleKnobPerRad(const std::string &key, unsigned int rad_id) {
  if (_double_knobs_per_rad[rad_id].find(key) == _double_knobs_per_rad[rad_id].end()) {
    std::cerr << "GetDoubleKnobPerRAD: Cannot find configuration parameter \"" << key
              << "\"" << std::endl;
    exit(1);
  }
  return _double_knobs_per_rad[rad_id][key];
}

// Gets the value of a string configuration knob
std::string RADSimConfig::GetStringKnobPerRad(const std::string &key, unsigned int rad_id) {
  if (_string_knobs_per_rad[rad_id].find(key) == _string_knobs_per_rad[rad_id].end()) {
    std::cerr << "GetStringKnobPerRAD: Cannot find configuration parameter \"" << key
              << "\"" << std::endl;
    exit(1);
  }
  return _string_knobs_per_rad[rad_id][key];
}

// Gets the value of an integer vector configuration knob
int RADSimConfig::GetIntVectorKnobPerRad(const std::string &key, unsigned int idx, unsigned int rad_id) {
  if (_int_vector_knobs_per_rad[rad_id].find(key) == _int_vector_knobs_per_rad[rad_id].end()) {
    std::cerr << "GetIntVectorKnobPerRAD: Cannot find configuration parameter \""
              << key << "\"" << std::endl;
    exit(1);
  }
  return _int_vector_knobs_per_rad[rad_id][key][idx];
}

// Gets the value of a double vector configuration knob
double RADSimConfig::GetDoubleVectorKnobPerRad(const std::string &key,
                                         unsigned int idx, unsigned int rad_id) {
  if (_double_vector_knobs_per_rad[rad_id].find(key) == _double_vector_knobs_per_rad[rad_id].end()) {
    std::cerr << "GetDoubleVectorKnobPerRAD: Cannot find configuration parameter \""
              << key << "\"" << std::endl;
    exit(1);
  }
  return _double_vector_knobs_per_rad[rad_id][key][idx];
}

// Gets the value of a string vector configuration knob
std::string RADSimConfig::GetStringVectorKnobPerRad(const std::string &key,
                                              unsigned int idx, unsigned int rad_id) {
  if (_string_vector_knobs_per_rad[rad_id].find(key) == _string_vector_knobs_per_rad[rad_id].end()) {
    std::cerr << "GetStringVectorKnobPerRAD: Cannot find configuration parameter \""
              << key << "\"" << std::endl;
    exit(1);
  }
  // if (key == "dram_config_files" ) {
  //   std::cout << "radsim_config.cpp: dram_config_files: " << std::endl;
  //   for (unsigned int i = 0; i < _string_vector_knobs_per_rad[rad_id][key].size(); i++) {
  //     std::cout << _string_vector_knobs_per_rad[rad_id][key][i] << std::endl;
  //   }
  // }
  return _string_vector_knobs_per_rad[rad_id][key][idx];
}

// Gets the value of an integer vector configuration knob
std::vector<int> &RADSimConfig::GetIntVectorKnobPerRad(const std::string &key, unsigned int rad_id) {
  if (_int_vector_knobs_per_rad[rad_id].find(key) == _int_vector_knobs_per_rad[rad_id].end()) {
    std::cerr << "GetIntVectorKnobPerRAD: Cannot find configuration parameter \""
              << key << "\"" << std::endl;
    exit(1);
  }
  return _int_vector_knobs_per_rad[rad_id][key];
}

// Gets the value of a double vector configuration knob
std::vector<double> &RADSimConfig::GetDoubleVectorKnobPerRad(const std::string &key, unsigned int rad_id) {
  if (_double_vector_knobs_per_rad[rad_id].find(key) == _double_vector_knobs_per_rad[rad_id].end()) {
    std::cerr << "GetDoubleVectorKnobPerRAD: Cannot find configuration parameter \""
              << key << "\"" << std::endl;
    exit(1);
  }
  return _double_vector_knobs_per_rad[rad_id][key];
}

// Gets the value of a string vector configuration knob
std::vector<std::string> &
RADSimConfig::GetStringVectorKnobPerRad(const std::string &key, unsigned int rad_id) {
  if (_string_vector_knobs_per_rad[rad_id].find(key) == _string_vector_knobs_per_rad[rad_id].end()) {
    std::cerr << "GetStringVectorKnobPerRAD: Cannot find configuration parameter \""
              << key << "\"" << std::endl;
    exit(1);
  }
  return _string_vector_knobs_per_rad[rad_id][key];
}

// Check if an integer configuration knob is defined
bool RADSimConfig::HasIntKnobShared(const std::string &key) {
  return (_int_knobs_shared.find(key) != _int_knobs_shared.end());
}

// Check if a double configuration knob is defined
bool RADSimConfig::HasDoubleKnobShared(const std::string &key) {
  return (_double_knobs_shared.find(key) != _double_knobs_shared.end());
}

// Check if a string configuration knob is defined
bool RADSimConfig::HasStringKnobShared(const std::string &key) {
  return (_string_knobs_shared.find(key) != _string_knobs_shared.end());
}

// Check if an integer vector configuration knob is defined
bool RADSimConfig::HasIntVectorKnobShared(const std::string &key) {
  return (_int_vector_knobs_shared.find(key) != _int_vector_knobs_shared.end());
}

// Check if a double vector configuration knob is defined
bool RADSimConfig::HasDoubleVectorKnobShared(const std::string &key) {
  return (_double_vector_knobs_shared.find(key) != _double_vector_knobs_shared.end());
}

// Check if a string vector configuration knob is defined
bool RADSimConfig::HasStringVectorKnobShared(const std::string &key) {
  return (_string_vector_knobs_shared.find(key) != _string_vector_knobs_shared.end());
}

//per-RAD functions to check if has certain knob defined for that RAD
// Check if an integer configuration knob is defined
bool RADSimConfig::HasIntKnobPerRad(const std::string &key, unsigned int rad_id) {
  return (_int_knobs_per_rad[rad_id].find(key) != _int_knobs_per_rad[rad_id].end());
}

// Check if a double configuration knob is defined
bool RADSimConfig::HasDoubleKnobPerRad(const std::string &key, unsigned int rad_id) {
  return (_double_knobs_per_rad[rad_id].find(key) != _double_knobs_per_rad[rad_id].end());
}

// Check if a string configuration knob is defined
bool RADSimConfig::HasStringKnobPerRad(const std::string &key, unsigned int rad_id) {
  return (_string_knobs_per_rad[rad_id].find(key) != _string_knobs_per_rad[rad_id].end());
}

// Check if an integer vector configuration knob is defined
bool RADSimConfig::HasIntVectorKnobPerRad(const std::string &key, unsigned int rad_id) {
  return (_int_vector_knobs_per_rad[rad_id].find(key) != _int_vector_knobs_per_rad[rad_id].end());
}

// Check if a double vector configuration knob is defined
bool RADSimConfig::HasDoubleVectorKnobPerRad(const std::string &key, unsigned int rad_id) {
  return (_double_vector_knobs_per_rad[rad_id].find(key) != _double_vector_knobs_per_rad[rad_id].end());
}

// Check if a string vector configuration knob is defined
bool RADSimConfig::HasStringVectorKnobPerRad(const std::string &key, unsigned int rad_id) {
  return (_string_vector_knobs_per_rad[rad_id].find(key) != _string_vector_knobs_per_rad[rad_id].end());
}

// Parse RADSim knobs from file into RADSimConfig data structures
void ParseRADSimKnobs(const std::string &knobs_filename) {
  std::ifstream knobs_file(knobs_filename);
  std::string line;

  double max_period = 0.0;

  while (std::getline(knobs_file, line)) {
    // Form a string stream out of a line
    std::stringstream ss(line);

    // Get parameter name
    std::string param;
    std::getline(ss, param, ' ');

    // Based on parameter name, parse a single or a vector of values of int,
    // double or string data types
    if ( (param == "radsim_root_dir") || (param == "cluster_topology") ){ //TODO: support other topologies, for now this param does not do anything actively
      std::string value;
      std::getline(ss, value, ' ');
      radsim_config.AddStringKnobShared(param, value);
    } else if ((param == "radsim_user_design_root_dir") || (param == "design_name")) { //example: design_name 2 dlrm
      std::string rad_id_str;
      std::getline(ss, rad_id_str, ' ');
      unsigned int rad_id = std::stoi(rad_id_str);
      std::string value;
      std::getline(ss, value, ' ');
      radsim_config.AddStringKnobPerRad(param, value, rad_id);
    } else if ( (param == "telemetry_log_verbosity") || (param == "num_rads") || (param == "inter_rad_latency_cycles") || (param == "inter_rad_fifo_num_slots") 
      || (param == "inter_rad_bw_accept_cycles") || (param == "inter_rad_bw_total_cycles") ) {
      std::string value_str;
      std::getline(ss, value_str, ' ');
      int value = std::stoi(value_str);
      radsim_config.AddIntKnobShared(param, value);
    } else if ((param == "noc_num_nocs") || (param == "dram_num_controllers")) {
      std::string rad_id_str;
      std::getline(ss, rad_id_str, ' ');
      unsigned int rad_id = std::stoi(rad_id_str);
      std::string value_str;
      std::getline(ss, value_str, ' ');
      int value = std::stoi(value_str);
      radsim_config.AddIntKnobPerRad(param, value, rad_id);
    } else if ((param == "noc_payload_width") || (param == "noc_vcs") ||
               (param == "noc_num_nodes") || (param == "noc_adapters_fifo_size") ||
               (param == "noc_adapters_obuff_size") ||
               (param == "dram_queue_sizes")) {
      //get rad-id
      std::string rad_id_str;
      std::getline(ss, rad_id_str, ' ');
      unsigned int rad_id = std::stoi(rad_id_str);
      //get values
      std::vector<int> value;
      std::string value_element_str;
      int value_element;
      while (getline(ss, value_element_str, ' ')) {
        value_element = std::stoi(value_element_str);
        value.push_back(value_element);
      }
      radsim_config.AddIntVectorKnobPerRad(param, value, rad_id);
    } else if ((param == "sim_driver_period")) {
      std::string value_element_str;
      std::getline(ss, value_element_str, ' ');
      double value = std::stod(value_element_str);
      if (value > max_period) {
        max_period = value;
      }
      radsim_config.AddDoubleKnobShared(param, value);
    } else if ((param == "noc_clk_period") || (param == "noc_adapters_clk_period") ||
               (param == "design_clk_periods") ||
               (param == "dram_clk_periods")) {
      //get rad-id
      std::string rad_id_str;
      std::getline(ss, rad_id_str, ' ');
      unsigned int rad_id = std::stoi(rad_id_str);
      //get values
      std::vector<double> value;
      std::string value_element_str;
      double value_element;
      while (getline(ss, value_element_str, ' ')) {
        value_element = std::stod(value_element_str);
        if (value_element > max_period) {
          max_period = value_element;
        }
        value.push_back(value_element);
      }
      radsim_config.AddDoubleVectorKnobPerRad(param, value, rad_id);
    } else if (param == "telemetry_traces") {
      std::vector<std::string> value;
      std::string value_element;
      while (getline(ss, value_element, ' ')) {
        value.push_back(value_element);
      }
      radsim_config.AddStringVectorKnobShared(param, value);
      radsim_config.AddIntKnobShared("telemetry_num_traces", value.size());
    } else if ((param == "design_noc_placement") ||
               (param == "noc_adapters_in_arbiter") ||
               (param == "noc_adapters_out_arbiter") ||
               (param == "noc_adapters_vc_mapping") ||
               (param == "dram_config_files")) {
      //get rad-id
      std::string rad_id_str;
      std::getline(ss, rad_id_str, ' ');
      unsigned int rad_id = std::stoi(rad_id_str);
      //get values
      std::vector<std::string> value;
      std::string value_element;
      while (getline(ss, value_element, ' ')) {
        if (param == "dram_config_files") {
          //std::cout << "radsim_config.cpp: " << value_element << std::endl;
        }
        value.push_back(value_element);
      }
      radsim_config.AddStringVectorKnobPerRad(param, value, rad_id);
    } else if (param == "cluster_configs") {
      continue; //go to next iteration, not using this knob for anything currently. was used to generate the radsim_knobs file.
    }
    else {
      std::cerr << "Undefined RADSim knob \"" << param << "\"" << std::endl;
      exit(1);
    }
  }
}