#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

// Class for storing all RADSim configuration knobs
class RADSimConfig {
 public:
  // Simulation configuration parameters are stored in pairs of knob name and value
  static std::unordered_map<std::string, int> _int_knobs;
  static std::unordered_map<std::string, double> _double_knobs;
  static std::unordered_map<std::string, std::string> _string_knobs;

  RADSimConfig();
  ~RADSimConfig();
  static void AddIntKnob(const std::string& key, int val);
  static void AddDoubleKnob(const std::string& key, double val);
  static void AddStringKnob(const std::string& key, const std::string& val);
  static int GetIntKnob(const std::string& key);
  static double GetDoubleKnob(const std::string& key);
  static std::string GetStringKnob(const std::string& key);
  static bool HasIntKnob(const std::string& key);
  static bool HasDoubleKnob(const std::string& key);
  static bool HasStringKnob(const std::string& key);
};