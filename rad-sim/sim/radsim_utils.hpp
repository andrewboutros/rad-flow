#pragma once

#include <systemc.h>
#include <radsim_config.hpp>

struct AdapterInfo {
  bool _is_master = false;
  unsigned int _noc_id = 0;
  unsigned int _node_id = 0;
  unsigned int _adapter_clk_idx = 0;
  unsigned int _module_clk_idx = 0;
  std::vector<std::string> _port_names;
  std::vector<unsigned int> _port_types;
  std::vector<unsigned int> _port_dataw;
};

// Returns the current simulation cycle number given a certain clock period
int GetSimulationCycle(double period);

int GetSimulationCycle();

// Returns the current simulation cycle number given the NoC clock period
int GetSimTime();

// Calculates the number of hops between source and destination nodes in a 2D grid of certain dimension in case of
// minimal XY routing
int CalculateXYGridHops(unsigned int src, unsigned int dest, unsigned int grid_dimension);