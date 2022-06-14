#include "radsim_config.hpp"

RADSimConfig::RADSimConfig() {}
RADSimConfig::~RADSimConfig() {}

// Adds a new integer configuration knob
void RADSimConfig::AddIntKnob(const std::string& key, int val) { 
  _int_knobs[key] = val; 
}

// Adds a new double configuration knob
void RADSimConfig::AddDoubleKnob(const std::string& key, double val) { 
  _double_knobs[key] = val; 
}

// Adds a new string configuration knob
void RADSimConfig::AddStringKnob(const std::string& key, std::string& val) { 
  _string_knobs[key] = val; 
}

// Adds a new integer vector configuration knob
void RADSimConfig::AddIntVectorKnob(const std::string& key, std::vector<int>& val) { 
  _int_vector_knobs[key] = val; 
}

// Adds a new double vector configuration knob
void RADSimConfig::AddDoubleVectorKnob(const std::string& key, std::vector<double>& val) { 
  _double_vector_knobs[key] = val; 
}

// Adds a new string vector configuration knob
void RADSimConfig::AddStringVectorKnob(const std::string& key, std::vector<std::string>& val) { 
  _string_vector_knobs[key] = val; 
}

// Gets the value of an integer configuration knob
int RADSimConfig::GetIntKnob(const std::string& key) {
  if (_int_knobs.find(key) == _int_knobs.end()) {
    std::cerr << "Cannot find configuration parameter \"" << key << "\"" << std::endl;
    exit(1);
  }
  return _int_knobs[key];
}

// Gets the value of a double configuration knob
double RADSimConfig::GetDoubleKnob(const std::string& key) {
  if (_double_knobs.find(key) == _double_knobs.end()) {
    std::cerr << "Cannot find configuration parameter \"" << key << "\"" << std::endl;
    exit(1);
  }
  return _double_knobs[key];
}

// Gets the value of a string configuration knob
std::string RADSimConfig::GetStringKnob(const std::string& key) {
  if (_string_knobs.find(key) == _string_knobs.end()) {
    std::cerr << "Cannot find configuration parameter \"" << key << "\"" << std::endl;
    exit(1);
  }
  return _string_knobs[key];
}

// Gets the value of an integer vector configuration knob
int RADSimConfig::GetIntVectorKnob(const std::string& key, unsigned int idx) {
  if (_int_vector_knobs.find(key) == _int_vector_knobs.end()) {
    std::cerr << "Cannot find configuration parameter \"" << key << "\"" << std::endl;
    exit(1);
  }
  return _int_vector_knobs[key][idx];
}

// Gets the value of a double vector configuration knob
double RADSimConfig::GetDoubleVectorKnob(const std::string& key, unsigned int idx) {
  if (_double_vector_knobs.find(key) == _double_vector_knobs.end()) {
    std::cerr << "Cannot find configuration parameter \"" << key << "\"" << std::endl;
    exit(1);
  }
  return _double_vector_knobs[key][idx];
}

// Gets the value of a string vector configuration knob
std::string RADSimConfig::GetStringVectorKnob(const std::string& key, unsigned int idx) {
  if (_string_vector_knobs.find(key) == _string_vector_knobs.end()) {
    std::cerr << "Cannot find configuration parameter \"" << key << "\"" << std::endl;
    exit(1);
  }
  return _string_vector_knobs[key][idx];
}

// Gets the value of an integer vector configuration knob
std::vector<int>& RADSimConfig::GetIntVectorKnob(const std::string& key) {
  if (_int_vector_knobs.find(key) == _int_vector_knobs.end()) {
    std::cerr << "Cannot find configuration parameter \"" << key << "\"" << std::endl;
    exit(1);
  }
  return _int_vector_knobs[key];
}

// Gets the value of a double vector configuration knob
std::vector<double>& RADSimConfig::GetDoubleVectorKnob(const std::string& key) {
  if (_double_vector_knobs.find(key) == _double_vector_knobs.end()) {
    std::cerr << "Cannot find configuration parameter \"" << key << "\"" << std::endl;
    exit(1);
  }
  return _double_vector_knobs[key];
}

// Gets the value of a string vector configuration knob
std::vector<std::string>& RADSimConfig::GetStringVectorKnob(const std::string& key) {
  if (_string_vector_knobs.find(key) == _string_vector_knobs.end()) {
    std::cerr << "Cannot find configuration parameter \"" << key << "\"" << std::endl;
    exit(1);
  }
  return _string_vector_knobs[key];
}

// Check if an integer configuration knob is defined
bool RADSimConfig::HasIntKnob(const std::string& key) { 
  return (_int_knobs.find(key) != _int_knobs.end()); 
}

// Check if a double configuration knob is defined
bool RADSimConfig::HasDoubleKnob(const std::string& key) { 
  return (_double_knobs.find(key) != _double_knobs.end()); 
}

// Check if a string configuration knob is defined
bool RADSimConfig::HasStringKnob(const std::string& key) { 
  return (_string_knobs.find(key) != _string_knobs.end()); 
}

// Check if an integer vector configuration knob is defined
bool RADSimConfig::HasIntVectorKnob(const std::string& key) { 
  return (_int_vector_knobs.find(key) != _int_vector_knobs.end()); 
}

// Check if a double vector configuration knob is defined
bool RADSimConfig::HasDoubleVectorKnob(const std::string& key) { 
  return (_double_vector_knobs.find(key) != _double_vector_knobs.end()); 
}

// Check if a string vector configuration knob is defined
bool RADSimConfig::HasStringVectorKnob(const std::string& key) { 
  return (_string_vector_knobs.find(key) != _string_vector_knobs.end()); 
}

// Parse RADSim knobs from file into RADSimConfig data structures
void ParseRADSimKnobs(const std::string& knobs_filename) {
  std::ifstream knobs_file(knobs_filename);
  std::string line;

  double max_period = 0.0;

  while (std::getline(knobs_file, line)) {
    // Form a string stream out of a line
    std::stringstream ss(line);

    // Get parameter name
    std::string param;
    std::getline(ss, param, ' ');

    // Based on parameter name, parse a single or a vector of values of int, double or string data types
    if ((param == "radsim_root_dir") ||
        (param == "radsim_user_design_root_dir")){
      std::string value;
      std::getline(ss, value, ' ');
      radsim_config.AddStringKnob(param, value);
    } else if ((param == "num_nocs") ||
               (param == "log_verbosity") ||
               (param == "num_traces") ||
               (param == "num_trace_modules")) {
      std::string value_str;
      std::getline(ss, value_str, ' ');
      int value = std::stoi(value_str);
      radsim_config.AddIntKnob(param, value);
    } else if ((param == "noc_payload_width") ||
               (param == "noc_vcs") ||
               (param == "noc_num_nodes") ||
               (param == "adapter_fifo_size") ||
               (param == "adapter_obuff_size")) {
      std::vector<int> value;
      std::string value_element_str;
      int value_element;
      while (getline(ss, value_element_str, ' ')) {
        value_element = std::stoi(value_element_str);
        value.push_back(value_element);
      }
      radsim_config.AddIntVectorKnob(param, value);
    } else if ((param == "sim_driver_period")) {
      std::string value_element_str;
      std::getline(ss, value_element_str, ' ');
      double value = std::stod(value_element_str);
      if (value > max_period) {
        max_period = value;
      }
      radsim_config.AddDoubleKnob(param, value);
    } else if ((param == "noc_period") ||
               (param == "adapter_period") ||
               (param == "module_period")) {
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
      radsim_config.AddDoubleVectorKnob(param, value);
    } else if ((param == "noc_placement_file") ||
               (param == "adapter_in_arbiter") ||
               (param == "adapter_out_arbiter") ||
               (param == "adapter_vc_mapping") ||
               (param == "trace_names")) {
      std::vector<std::string> value;
      std::string value_element;
      while (getline(ss, value_element, ' ')) {
        value.push_back(value_element);
      }
      radsim_config.AddStringVectorKnob(param, value);
    } else {
      std::cerr << "Undefined RADSim knob \"" << param << "\"" << std::endl;
      exit(1);
    }
  }
  radsim_config.AddDoubleKnob("max_period", max_period);
}