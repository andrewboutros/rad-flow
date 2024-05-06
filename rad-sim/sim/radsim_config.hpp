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
  //AKB: appended _shared to names
  std::unordered_map<std::string, int> _int_knobs_shared;
  std::unordered_map<std::string, double> _double_knobs_shared;
  std::unordered_map<std::string, std::string> _string_knobs_shared;
  std::unordered_map<std::string, std::vector<int>> _int_vector_knobs_shared;
  std::unordered_map<std::string, std::vector<double>> _double_vector_knobs_shared;
  std::unordered_map<std::string, std::vector<std::string>> _string_vector_knobs_shared;

  //AKB: for rad-specific parameters
  std::vector<std::unordered_map<std::string, int>> _int_knobs_per_rad;
  std::vector<std::unordered_map<std::string, double>> _double_knobs_per_rad;
  std::vector<std::unordered_map<std::string, std::string>> _string_knobs_per_rad;
  std::vector<std::unordered_map<std::string, std::vector<int>>> _int_vector_knobs_per_rad;
  std::vector<std::unordered_map<std::string, std::vector<double>>> _double_vector_knobs_per_rad;
  std::vector<std::unordered_map<std::string, std::vector<std::string>>> _string_vector_knobs_per_rad;

  RADSimConfig();
  ~RADSimConfig();
  //AKB: changed to be general parameters aka shared across all RADs
  void AddIntKnobShared(const std::string& key, int val);
  void AddDoubleKnobShared(const std::string& key, double val);
  void AddStringKnobShared(const std::string& key, std::string& val);
  void AddIntVectorKnobShared(const std::string& key, std::vector<int>& val);
  void AddDoubleVectorKnobShared(const std::string& key, std::vector<double>& val);
  void AddStringVectorKnobShared(const std::string& key, std::vector<std::string>& val);
  //AKB: rad-specific parameters
  void AddIntKnobPerRad(const std::string& key, int val, int rad_id);
  void AddDoubleKnobPerRad(const std::string& key, double val, int rad_id);
  void AddStringKnobPerRad(const std::string& key, std::string& val, int rad_id);
  void AddIntVectorKnobPerRad(const std::string& key, std::vector<int>& val, int rad_id);
  void AddDoubleVectorKnobPerRad(const std::string& key, std::vector<double>& val, int rad_id);
  void AddStringVectorKnobPerRad(const std::string& key, std::vector<std::string>& val, int rad_id);
  //AKB: changed to be parameters shared across all RADs
  int GetIntKnobShared(const std::string& key);
  double GetDoubleKnobShared(const std::string& key);
  std::string GetStringKnobShared(const std::string& key);
  int GetIntVectorKnobShared(const std::string& key, unsigned int idx);
  double GetDoubleVectorKnobShared(const std::string& key, unsigned int idx);
  std::string GetStringVectorKnobShared(const std::string& key, unsigned int idx);
  std::vector<int>& GetIntVectorKnobShared(const std::string& key);
  std::vector<double>& GetDoubleVectorKnobShared(const std::string& key);
  std::vector<std::string>& GetStringVectorKnobShared(const std::string& key);
  //AKB: rad-specific parameters
  int GetIntKnobPerRad(const std::string& key, int rad_id);
  double GetDoubleKnobPerRad(const std::string& key, int rad_id);
  std::string GetStringKnobPerRad(const std::string& key, int rad_id);
  int GetIntVectorKnobPerRad(const std::string& key, unsigned int idx, int rad_id);
  double GetDoubleVectorKnobPerRad(const std::string& key, unsigned int idx, int rad_id);
  std::string GetStringVectorKnobPerRad(const std::string& key, unsigned int idx, int rad_id);
  std::vector<int>& GetIntVectorKnobPerRad(const std::string& key, int rad_id);
  std::vector<double>& GetDoubleVectorKnobPerRad(const std::string& key, int rad_id);
  std::vector<std::string>& GetStringVectorKnobPerRad(const std::string& key, int rad_id);
  bool HasIntKnob(const std::string& key);
  bool HasDoubleKnob(const std::string& key);
  bool HasStringKnob(const std::string& key);
  bool HasIntVectorKnob(const std::string& key);
  bool HasDoubleVectorKnob(const std::string& key);
  bool HasStringVectorKnob(const std::string& key);
};

void ParseRADSimKnobs(const std::string& knobs_filename);

extern RADSimConfig radsim_config;