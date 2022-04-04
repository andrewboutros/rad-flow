#include "radsim_config.hpp"

std::unordered_map<std::string, int> RADSimConfig::_int_knobs;
std::unordered_map<std::string, double> RADSimConfig::_double_knobs;
std::unordered_map<std::string, std::string> RADSimConfig::_string_knobs;

RADSimConfig::RADSimConfig() {}
RADSimConfig::~RADSimConfig() {}

// Adds a new integer configuration knob
void RADSimConfig::AddIntKnob(const std::string& key, int val) { _int_knobs[key] = val; }

// Adds a new double configuration knob
void RADSimConfig::AddDoubleKnob(const std::string& key, double val) { _double_knobs[key] = val; }

// Adds a new string configuration knob
void RADSimConfig::AddStringKnob(const std::string& key, const std::string& val) { _string_knobs[key] = val; }

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

// Check if an integer configuration knob is defined
bool RADSimConfig::HasIntKnob(const std::string& key) { return (_int_knobs.find(key) != _int_knobs.end()); }

// Check if a double configuration knob is defined
bool RADSimConfig::HasDoubleKnob(const std::string& key) { return (_double_knobs.find(key) != _double_knobs.end()); }

// Check if a string configuration knob is defined
bool RADSimConfig::HasStringKnob(const std::string& key) { return (_string_knobs.find(key) != _string_knobs.end()); }