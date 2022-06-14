#include <radsim_utils.hpp>

int GetSimulationCycle(double period) {
  sc_time t = sc_time_stamp();
  int cycle = (int)ceil(t.value() / period / 1000);
  return cycle;
}

int GetSimulationCycle() {
  double period = radsim_config.GetDoubleKnob("max_period");
  sc_time t = sc_time_stamp();
  int cycle = (int)ceil(t.value() / period / 1000);
  return cycle;
}

int GetSimTime() { 
  sc_time t = sc_time_stamp();
  return t.value();
}

int CalculateXYGridHops(unsigned int src, unsigned int dest, unsigned int grid_dimension) {
  int grid_size = grid_dimension * grid_dimension;
  int src_node = src % grid_size;
  int src_row = src_node / grid_dimension;
  int src_col = src_node % grid_dimension;

  int dst_node = dest % grid_size;
  int dst_row = dst_node / grid_dimension;
  int dst_col = dst_node % grid_dimension;

  return (abs(src_row - dst_row) + abs(src_col - dst_col) + 1);
}