#include <traffic_generator.hpp>

// read/write  from memory <-> memory controller
// = to generator 
traffic_generator::traffic_generator(
    const sc_module_name &name, unsigned int dataw,
    std::vector<unsigned int> &num_mem_channels_per_controller,
    unsigned int fifo_depth)
    : RADSimModule(name) {

  _total_num_channels = 0;
  unsigned int ctrl_id = 0;
  for (auto &num_channels : num_mem_channels_per_controller) {
    _num_channels_per_ctrl.push_back(num_channels);
    _total_num_channels += num_channels;
    for (unsigned int ch_id = 0; ch_id < num_channels; ch_id++) {
      std::string port_name =
          "ext_mem_" + to_string(ctrl_id) + ".mem_channel_" + to_string(ch_id);
      _dst_port_names.push_back(port_name);
    }
    ctrl_id++;
  }
  _lookup_indecies_fifo.resize(_total_num_channels);
  _base_addresses_fifo.resize(_total_num_channels);
  _fifo_depth = fifo_depth;
  _fifo_full.init(_total_num_channels);
  _id_count.init(_total_num_channels);
  _num_received_responses = 0;
  _dataw = dataw;

  aximm_req_interface.init(_total_num_channels);

  // Combinational logic and its sensitivity list
  SC_METHOD(Assign);
  sensitive << rst;
  for (unsigned int ch_id = 0; ch_id < _total_num_channels; ch_id++) {
    sensitive << _fifo_full[ch_id];
  }
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
  _debug_sent_request_counter = 0;
}

traffic_generator::~traffic_generator() {}

void traffic_generator::Assign() {
  if (rst) {
    lookup_indecies_ready.write(true);
    for (unsigned int ch_id = 0; ch_id < _total_num_channels; ch_id++) {
      aximm_req_interface[ch_id].bready.write(false);
      aximm_req_interface[ch_id].rready.write(false);
    }
  } else {
    bool all_fifos_not_full = true;

    // Always ready to accept read/write response from the AXI-MM NoC
    // interface
    for (unsigned int ch_id = 0; ch_id < _total_num_channels; ch_id++) {
      aximm_req_interface[ch_id].bready.write(true);
      aximm_req_interface[ch_id].rready.write(true);
      all_fifos_not_full = all_fifos_not_full && !_fifo_full[ch_id].read();
    }

    // Ready to accept new lookup indecies from driver testbench as long as
    // none of the FIFOs are full
    lookup_indecies_ready.write(all_fifos_not_full);
  }
}

void traffic_generator::Tick() {
  // Reset logic
  for (unsigned int ch_id = 0; ch_id < _total_num_channels; ch_id++) {
    aximm_req_interface[ch_id].arvalid.write(false);
    aximm_req_interface[ch_id].awvalid.write(false);
    aximm_req_interface[ch_id].wvalid.write(false);
    while (!_lookup_indecies_fifo[ch_id].empty()) {
      _lookup_indecies_fifo[ch_id].pop();
    }
    while (!_base_addresses_fifo[ch_id].empty()) {
      _base_addresses_fifo[ch_id].pop();
    }
    _fifo_full[ch_id].write(false);
    _id_count[ch_id].write(0);
  }
  wait();

  // Always @ positive edge of the clock
  while (true) {
    // Interface with testbench driver
    if (lookup_indecies_ready.read() && lookup_indecies_valid.read()) {
      data_vector<uint64_t> lookup_indecies = lookup_indecies_data.read();
      data_vector<unsigned int> target_channels =
          lookup_indecies_target_channels.read();
      data_vector<uint64_t> base_addresses =
          lookup_indecies_base_addresses.read();
      for (unsigned int i = 0; i < lookup_indecies.size(); i++) {
        _lookup_indecies_fifo[target_channels[i]].push(lookup_indecies[i]);
        _base_addresses_fifo[target_channels[i]].push(base_addresses[i]);
      }
      // std::cout << module_name << ": Received lookup indecies" << std::endl;
    }

    // Set FIFO full signals
    for (unsigned int ch_id = 0; ch_id < _total_num_channels; ch_id++) {
      _fifo_full[ch_id].write(_lookup_indecies_fifo[ch_id].size() >=
                              _fifo_depth - 4);
    }

    // Sending transactions to AXI-MM NoC
    unsigned int ch_id = 0;
    for (unsigned int ctrl_id = 0; ctrl_id < _num_channels_per_ctrl.size();
         ctrl_id++) {
      for (unsigned int c = 0; c < _num_channels_per_ctrl[ctrl_id]; c++) {
        if (!_lookup_indecies_fifo[ch_id].empty()) {
          uint64_t lookup_index = _lookup_indecies_fifo[ch_id].front();
          uint64_t table_base_addr = _base_addresses_fifo[ch_id].front();

          std::string dst_port_name = _dst_port_names[ch_id];
          uint64_t dst_addr = radsim_design.GetPortBaseAddress(dst_port_name) +
                              table_base_addr + lookup_index;
          std::string src_port_name =
              "feature_interaction_inst.aximm_interface_" +
              std::to_string(ch_id);
          uint64_t src_addr = radsim_design.GetPortBaseAddress(src_port_name);

          /*if (ctrl_id == 0) {
            std::cout << "Base address: " << table_base_addr << std::endl;
            std::cout << "Index: " << lookup_index << std::endl;
          }*/

          aximm_req_interface[ch_id].araddr.write(dst_addr);
          aximm_req_interface[ch_id].arid.write(_id_count[ch_id].read());
          aximm_req_interface[ch_id].arlen.write(0);
          aximm_req_interface[ch_id].arburst.write(0);
          aximm_req_interface[ch_id].arsize.write(0);
          aximm_req_interface[ch_id].aruser.write(src_addr);
          aximm_req_interface[ch_id].arvalid.write(true);
          aximm_req_interface[ch_id].awvalid.write(false);
          aximm_req_interface[ch_id].wvalid.write(false);
        } else {
          aximm_req_interface[ch_id].arvalid.write(false);
          aximm_req_interface[ch_id].awvalid.write(false);
          aximm_req_interface[ch_id].wvalid.write(false);
        }

        // Pop the FIFO if the transaction is accepted
        if (aximm_req_interface[ch_id].arvalid.read() &&
            aximm_req_interface[ch_id].arready.read()) {
          /*if (ctrl_id == 0) {
            std::cout << "ELU sent address "
                      << aximm_req_interface[ch_id].araddr.read().to_uint64()
                      << std::endl;
            cin.get();
          }*/
          _lookup_indecies_fifo[ch_id].pop();
          _base_addresses_fifo[ch_id].pop();
          _id_count[ch_id].write(_id_count[ch_id].read() + 1);
          /*_debug_sent_request_counter++;
          std::cout << module_name << ": Sent AR transaction "
                    << _debug_sent_request_counter << " @ channel " << ch_id
                    << "!" << std::endl;*/
        }

        // Receiving transactions from AXI-MM NoC
        if (aximm_req_interface[ch_id].rvalid.read() &&
            aximm_req_interface[ch_id].rready.read()) {
          /*std::cout << module_name << ": Received READ response "
                    << _num_received_responses << " ("
                    << aximm_req_interface[ch_id].rdata.read() << ")!"
                    << std::endl;*/
          _num_received_responses++;
        } else if (aximm_req_interface[ch_id].bvalid.read() &&
                   aximm_req_interface[ch_id].bready.read()) {
          // std::cout << module_name << ": Received WRITE response!" <<
          // std::endl;
          _num_received_responses++;
        }
        ch_id++;
      }
    }
    wait();
  }
}

void traffic_generator::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;

  for (unsigned int ch_id = 0; ch_id < _total_num_channels; ch_id++) {
    port_name = module_name + ".aximm_req_interface_" + std::to_string(ch_id);
    // std::cout << "----" << port_name << std::endl;
    RegisterAximmMasterPort(port_name, &aximm_req_interface[ch_id], _dataw);
  }
}
