#include "axis_switch.hpp"

axis_switch::axis_switch(const sc_module_name& _name, unsigned int _num_ouptut_interfaces,
                         std::unordered_map<unsigned int, unsigned int> _dest_to_interface_id)
    : sc_module(_name) {
  num_output_interfaces = _num_ouptut_interfaces;
  dest_to_interface_id = _dest_to_interface_id;

  // Size the vector of output AXI-streaming master interfaces
  init_vector<axis_master_port>::init_sc_vector(switch_master_interfaces, num_output_interfaces);

  // Set sensitivity list of SC_METHOD & clock and reset of SC_CTHREAD
  SC_METHOD(Assign);
  sensitive << rst << switch_slave_interface.tdest;
  for (unsigned int interface_id = 0; interface_id < num_output_interfaces; interface_id++)
    sensitive << switch_master_interfaces[interface_id].tready;
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

axis_switch::~axis_switch() {}

void axis_switch::Tick() {
  // Reset logic
  for (unsigned int interface_id = 0; interface_id < num_output_interfaces; interface_id++) {
    switch_master_interfaces[interface_id].tvalid.write(false);
  }
  wait();

  // Sequential logic
  while (true) {
    // Set tvalid of all output interfaces to false -- only one of them might later be set to true
    for (unsigned int interface_id = 0; interface_id < num_output_interfaces; interface_id++) {
      switch_master_interfaces[interface_id].tvalid.write(false);
    }

    // If an AXI-streaming transaction is performed on the slave interface (i.e., both tvalid and tready are high),
    // translate the incoming transfer destination to an interface ID and route transfer to the corresponding interface
    if (switch_slave_interface.tvalid.read() && switch_slave_interface.tready.read()) {
#ifdef RADSIM
      unsigned int destination = switch_slave_interface.tdest.read().to_uint();
#else
      unsigned int destination = switch_slave_interface.tdest.read().to_uint() % 6;
#endif
      unsigned int master_interface_index = dest_to_interface_id[destination];
      if (master_interface_index == 0)
        count_evrf++;
      else if (master_interface_index == 1)
        count_mfu0++;
      else if (master_interface_index == 2)
        count_mfu1++;
      // cout << "TDEST = " << switch_slave_interface.tdest.read().to_uint() << ", Interface = " <<
      // master_interface_index << endl; cout << this->name() << " " << count_evrf << " " << count_mfu0 << " " <<
      // count_mfu1 << endl;
      switch_master_interfaces[master_interface_index].tvalid.write(switch_slave_interface.tvalid);
      switch_master_interfaces[master_interface_index].tdata.write(switch_slave_interface.tdata);
      switch_master_interfaces[master_interface_index].tstrb.write(switch_slave_interface.tstrb);
      switch_master_interfaces[master_interface_index].tkeep.write(switch_slave_interface.tkeep);
      switch_master_interfaces[master_interface_index].tlast.write(switch_slave_interface.tlast);
      switch_master_interfaces[master_interface_index].tid.write(switch_slave_interface.tid);
      switch_master_interfaces[master_interface_index].tdest.write(switch_slave_interface.tdest);
      switch_master_interfaces[master_interface_index].tuser.write(switch_slave_interface.tuser);
    }
    wait();
  }
}

void axis_switch::Assign() {
  if (rst.read()) {
    switch_slave_interface.tready.write(false);
  } else {
    // Set the tready of the intended destination interface
    unsigned int destination = switch_slave_interface.tdest.read().to_uint();
    unsigned int master_interface_index = dest_to_interface_id[destination];
    switch_slave_interface.tready.write(switch_master_interfaces[master_interface_index].tready.read());
  }
}