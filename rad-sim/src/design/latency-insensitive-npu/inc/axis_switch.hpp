#pragma once

#include <systemc.h>
#include <axis_interface.hpp>
#include <axis_fifo_adapters.hpp>
#include <unordered_map>
#include <vector>

// This class defines an AXI-streaming switch which has one slave AXI-streaming interface and a number of master
// AXI-streaming interfaces -- depending on the destination of incoming transfers on the slave interface, they get
// routed to one of the master interfaces
class axis_switch : public sc_module {
 private:
  unsigned int num_output_interfaces;  // Number of master AXI-streaming interfaces
  std::unordered_map<unsigned int, unsigned int>
      dest_to_interface_id;  // Table for translating destination to interface ID

  int count_evrf = 0, count_mfu0 = 0, count_mfu1 = 0;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_vector<axis_master_port> switch_master_interfaces;
  axis_slave_port switch_slave_interface;

  axis_switch(const sc_module_name& _name, unsigned int _num_ouptut_interfaces,
              std::unordered_map<unsigned int, unsigned int> _dest_to_interface_id);
  ~axis_switch();

  void Assign();
  void Tick();
  SC_HAS_PROCESS(axis_switch);
};