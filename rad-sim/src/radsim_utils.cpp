#include <radsim_utils.hpp>

int GetSimulationCycle(double period) {
  sc_time t = sc_time_stamp();
  int cycle = (int)ceil(t.value() / period / 1000);
  return cycle;
}

int GetSimTime() { return GetSimulationCycle(NOC_PERIOD); }

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

int VCIDFromType(Flit::FlitType type, BookSimConfig* config){
  if (type == Flit::READ_REQUEST){
    return config->GetInt("read_request_begin_vc");
  } else if (type == Flit::WRITE_REQUEST){
    return config->GetInt("write_request_begin_vc");
  } else if (type == Flit::WRITE_DATA){
    return config->GetInt("write_data_begin_vc");
  } else if (type == Flit::READ_REPLY){
    return config->GetInt("read_reply_begin_vc");
  } else {
    return config->GetInt("write_reply_begin_vc");
  }
}