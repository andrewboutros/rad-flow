#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <radsim_telemetry.hpp>

// Class for storing all RADSim configuration knobs
class RADSimConfig {
 public:
  // Simulation configuration parameters are stored in pairs of knob name and value
  std::unordered_map<std::string, int> _int_knobs;
  std::unordered_map<std::string, double> _double_knobs;
  std::unordered_map<std::string, std::string> _string_knobs;
  std::unordered_map<std::string, std::vector<int>> _int_vector_knobs;
  std::unordered_map<std::string, std::vector<double>> _double_vector_knobs;
  std::unordered_map<std::string, std::vector<std::string>> _string_vector_knobs;

  RADSimConfig();
  ~RADSimConfig();
  void AddIntKnob(const std::string& key, int val);
  void AddDoubleKnob(const std::string& key, double val);
  void AddStringKnob(const std::string& key, std::string& val);
  void AddIntVectorKnob(const std::string& key, std::vector<int>& val);
  void AddDoubleVectorKnob(const std::string& key, std::vector<double>& val);
  void AddStringVectorKnob(const std::string& key, std::vector<std::string>& val);
  int GetIntKnob(const std::string& key);
  double GetDoubleKnob(const std::string& key);
  std::string GetStringKnob(const std::string& key);
  int GetIntVectorKnob(const std::string& key, unsigned int idx);
  double GetDoubleVectorKnob(const std::string& key, unsigned int idx);
  std::string GetStringVectorKnob(const std::string& key, unsigned int idx);
  std::vector<int>& GetIntVectorKnob(const std::string& key);
  std::vector<double>& GetDoubleVectorKnob(const std::string& key);
  std::vector<std::string>& GetStringVectorKnob(const std::string& key);
  bool HasIntKnob(const std::string& key);
  bool HasDoubleKnob(const std::string& key);
  bool HasStringKnob(const std::string& key);
  bool HasIntVectorKnob(const std::string& key);
  bool HasDoubleVectorKnob(const std::string& key);
  bool HasStringVectorKnob(const std::string& key);
};

void ParseRADSimKnobs(const std::string& knobs_filename);

extern RADSimConfig radsim_config;