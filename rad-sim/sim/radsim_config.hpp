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
  std::unordered_map<std::string, int> _int_knobs_shared;
  std::unordered_map<std::string, double> _double_knobs_shared;
  std::unordered_map<std::string, std::string> _string_knobs_shared;
  std::unordered_map<std::string, std::vector<int>> _int_vector_knobs_shared;
  std::unordered_map<std::string, std::vector<double>> _double_vector_knobs_shared;
  std::unordered_map<std::string, std::vector<std::string>> _string_vector_knobs_shared;

  //for rad-specific parameters
  std::vector<std::unordered_map<std::string, int>> _int_knobs_per_rad;
  std::vector<std::unordered_map<std::string, double>> _double_knobs_per_rad;
  std::vector<std::unordered_map<std::string, std::string>> _string_knobs_per_rad;
  std::vector<std::unordered_map<std::string, std::vector<int>>> _int_vector_knobs_per_rad;
  std::vector<std::unordered_map<std::string, std::vector<double>>> _double_vector_knobs_per_rad;
  std::vector<std::unordered_map<std::string, std::vector<std::string>>> _string_vector_knobs_per_rad;

  RADSimConfig();
  ~RADSimConfig();
  void ResizeAll(int num_rads); //resizing instance of class based on # of RADs
  //general parameters shared across all RADs
  void AddIntKnobShared(const std::string& key, int val);
  void AddDoubleKnobShared(const std::string& key, double val);
  void AddStringKnobShared(const std::string& key, std::string& val);
  void AddIntVectorKnobShared(const std::string& key, std::vector<int>& val);
  void AddDoubleVectorKnobShared(const std::string& key, std::vector<double>& val);
  void AddStringVectorKnobShared(const std::string& key, std::vector<std::string>& val);
  //rad-specific parameters
  void AddIntKnobPerRad(const std::string& key, int val, unsigned int rad_id);
  void AddDoubleKnobPerRad(const std::string& key, double val, unsigned int rad_id);
  void AddStringKnobPerRad(const std::string& key, std::string& val, unsigned int rad_id);
  void AddIntVectorKnobPerRad(const std::string& key, std::vector<int>& val, unsigned int rad_id);
  void AddDoubleVectorKnobPerRad(const std::string& key, std::vector<double>& val, unsigned int rad_id);
  void AddStringVectorKnobPerRad(const std::string& key, std::vector<std::string>& val, unsigned int rad_id);
  //general parameters shared across all RADs
  int GetIntKnobShared(const std::string& key);
  double GetDoubleKnobShared(const std::string& key);
  std::string GetStringKnobShared(const std::string& key);
  int GetIntVectorKnobShared(const std::string& key, unsigned int idx);
  double GetDoubleVectorKnobShared(const std::string& key, unsigned int idx);
  std::string GetStringVectorKnobShared(const std::string& key, unsigned int idx);
  std::vector<int>& GetIntVectorKnobShared(const std::string& key);
  std::vector<double>& GetDoubleVectorKnobShared(const std::string& key);
  std::vector<std::string>& GetStringVectorKnobShared(const std::string& key);
  //rad-specific parameters
  int GetIntKnobPerRad(const std::string& key, unsigned int rad_id);
  double GetDoubleKnobPerRad(const std::string& key, unsigned int rad_id);
  std::string GetStringKnobPerRad(const std::string& key, unsigned int rad_id);
  int GetIntVectorKnobPerRad(const std::string& key, unsigned int idx, unsigned int rad_id);
  double GetDoubleVectorKnobPerRad(const std::string& key, unsigned int idx, unsigned int rad_id);
  std::string GetStringVectorKnobPerRad(const std::string& key, unsigned int idx, unsigned int rad_id);
  std::vector<int>& GetIntVectorKnobPerRad(const std::string& key, unsigned int rad_id);
  std::vector<double>& GetDoubleVectorKnobPerRad(const std::string& key, unsigned int rad_id);
  std::vector<std::string>& GetStringVectorKnobPerRad(const std::string& key, unsigned int rad_id);
  //specify if shared knob
  bool HasIntKnobShared(const std::string& key);
  bool HasDoubleKnobShared(const std::string& key);
  bool HasStringKnobShared(const std::string& key);
  bool HasIntVectorKnobShared(const std::string& key);
  bool HasDoubleVectorKnobShared(const std::string& key);
  bool HasStringVectorKnobShared(const std::string& key);
  //rad-specific knobs
  bool HasIntKnobPerRad(const std::string& key, unsigned int rad_id);
  bool HasDoubleKnobPerRad(const std::string& key, unsigned int rad_id);
  bool HasStringKnobPerRad(const std::string& key, unsigned int rad_id);
  bool HasIntVectorKnobPerRad(const std::string& key, unsigned int rad_id);
  bool HasDoubleVectorKnobPerRad(const std::string& key, unsigned int rad_id);
  bool HasStringVectorKnobPerRad(const std::string& key, unsigned int rad_id);
};

void ParseRADSimKnobs(const std::string& knobs_filename);

extern RADSimConfig radsim_config;