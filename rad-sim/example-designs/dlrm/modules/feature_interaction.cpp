#include <feature_interaction.hpp>

void ParseFeatureInteractionInstructions(
    std::string &instructions_file,
    std::vector<feature_interaction_inst> &instructions,
    std::string &responses_file, unsigned int &num_expected_responses) {

  std::ifstream resp_file(responses_file);
  if (!resp_file) {
    sim_log.log(error, "Cannot find feature interaction responses file!");
  }
  std::string line;
  std::getline(resp_file, line);
  std::stringstream ls(line);
  unsigned int lookups, num_inputs;
  ls >> lookups >> num_inputs;
  num_expected_responses = lookups * num_inputs;
  resp_file.close();

  std::ifstream inst_file(instructions_file);
  if (!inst_file) {
    sim_log.log(error, "Cannot find feature interaction instructions file!");
  }

  unsigned int mux_select;
  std::string pop_signals;
  while (std::getline(inst_file, line)) {
    feature_interaction_inst instruction;
    std::stringstream line_stream(line);
    line_stream >> mux_select;
    instruction.mux_select = mux_select;
    line_stream >> pop_signals;
    uint8_t idx = 0;
    for (char &c : pop_signals) {
      if (c == '1') {
        instruction.fifo_pops.push_back(true);
      } else {
        instruction.fifo_pops.push_back(false);
      }
      idx++;
    }
    instructions.push_back(instruction);
  }
  inst_file.close();
}

feature_interaction::feature_interaction(const sc_module_name &name,
                                         unsigned int dataw,
                                         unsigned int element_bitwidth,
                                         unsigned int num_mem_channels,
                                         unsigned int fifos_depth,
                                         unsigned int num_output_channels,
                                         std::string &instructions_file)
    : radsim_module(name) {

  _fifos_depth = fifos_depth;
  _afifo_width_ratio_in = 32 / 4;
  _afifo_width_ratio_out = LANES / 4;
  _num_received_responses = 0;
  _num_mem_channels = num_mem_channels;
  _dataw = dataw;
  _bitwidth = element_bitwidth;
  _num_elements_wide_in = dataw / element_bitwidth;
  _num_elements_narrow = _num_elements_wide_in / _afifo_width_ratio_in;
  _num_elements_wide_out = _num_elements_narrow * _afifo_width_ratio_out;
  _num_output_channels = num_output_channels;
  _staging_counter = 0;
  _staging_data.resize(_num_elements_wide_out);

  aximm_interface.init(_num_mem_channels);
  axis_interface.init(_num_output_channels);

  _input_fifos.resize(_num_mem_channels);
  _ififo_full.init(_num_mem_channels);
  _ififo_empty.init(_num_mem_channels);

  _output_fifos.resize(_num_output_channels);
  _ofifo_full.init(_num_output_channels);
  _ofifo_empty.init(_num_output_channels);

  std::string resp_filename =
      radsim_config.GetStringKnob("radsim_user_design_root_dir") +
      "/compiler/embedding_indecies.in";
  ParseFeatureInteractionInstructions(instructions_file, _instructions,
                                      resp_filename, _num_expected_responses);

  // Combinational logic and its sensitivity list
  SC_METHOD(Assign);
  sensitive << rst;
  for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
    sensitive << _ififo_full[ch_id];
  }
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
  _debug_feature_interaction_out = new ofstream("dut_feature_interaction.out");
}

feature_interaction::~feature_interaction() {
  delete _debug_feature_interaction_out;
}

void feature_interaction::Assign() {
  if (rst) {
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      aximm_interface[ch_id].bready.write(false);
      aximm_interface[ch_id].rready.write(false);
    }
  } else {
    // Set ready signals to accept read/write response from the AXI-MM NoC
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      aximm_interface[ch_id].bready.write(false);
      aximm_interface[ch_id].rready.write(!_ififo_full[ch_id].read());
    }
  }
}

void feature_interaction::bv_to_data_vector(sc_bv<AXI4_MAX_DATAW> &bitvector,
                                            data_vector<int16_t> &datavector,
                                            unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * _bitwidth;
    end_idx = (e + 1) * _bitwidth;
    datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
  }
}

void feature_interaction::data_vector_to_bv(data_vector<int16_t> &datavector,
                                            sc_bv<AXIS_MAX_DATAW> &bitvector,
                                            unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * _bitwidth;
    end_idx = (e + 1) * _bitwidth;
    bitvector.range(end_idx - 1, start_idx) = datavector[e];
  }
}

bool are_ififos_ready(sc_vector<sc_signal<bool>> &ififo_empty,
                      feature_interaction_inst &inst) {

  bool ready = true;
  bool fifos_popped = (inst.mux_select > 0);
  for (unsigned int ch_id = 0; ch_id < inst.fifo_pops.size(); ch_id++) {
    if (inst.fifo_pops[ch_id]) {
      fifos_popped = true;
      ready &= !ififo_empty[ch_id].read();
    }
  }
  return (ready && fifos_popped);
}

void feature_interaction::Tick() {
  // Reset ports
  for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
    aximm_interface[ch_id].arvalid.write(false);
    aximm_interface[ch_id].awvalid.write(false);
    aximm_interface[ch_id].wvalid.write(false);
  }
  // feature_interaction_valid.write(false);
  received_responses.write(0);
  // Reset signals
  for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
    _ififo_full[ch_id].write(false);
    _ififo_empty[ch_id].write(true);
  }
  for (unsigned int ch_id = 0; ch_id < _num_output_channels; ch_id++) {
    _ofifo_full[ch_id].write(false);
    _ofifo_empty[ch_id].write(true);
    axis_interface[ch_id].tvalid.write(false);
  }
  _dest_ofifo.write(0);
  _src_ofifo.write(0);
  _pc.write(0);
  wait();

  // Always @ positive edge of the clock
  while (true) {
    // Accept R responses from the NoC
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      if (_input_fifos[ch_id].size() < _fifos_depth &&
          aximm_interface[ch_id].rvalid.read()) {
        sc_bv<AXI4_MAX_DATAW> rdata_bv = aximm_interface[ch_id].rdata.read();
        data_vector<int16_t> rdata(_num_elements_wide_in);
        bv_to_data_vector(rdata_bv, rdata, _num_elements_wide_in);
        for (unsigned int c = 0; c < _afifo_width_ratio_in; c++) {
          data_vector<int16_t> sliced_data(_num_elements_narrow);
          for (unsigned int e = 0; e < sliced_data.size(); e++) {
            sliced_data[e] = rdata[(c * sliced_data.size()) + e];
          }
          _input_fifos[ch_id].push(sliced_data);
        }
        _num_received_responses++;
        if (_num_received_responses == _num_expected_responses) {
          std::cout << this->name() << ": Got all memory responses at cycle "
                    << GetSimulationCycle(5.0) << "!" << std::endl;
        }
        // std::cout << GetSimulationCycle(5.0) << " === "
        //           << "Pushed response to iFIFO " << rdata << std::endl;
      }
    }

    // Pop from input FIFOs to staging register
    bool ififos_ready =
        are_ififos_ready(_ififo_empty, _instructions[_pc.read()]);
    if (ififos_ready && !_ofifo_full[_dest_ofifo.read()]) {
      // Pick the right iFIFO (or zeros) to push to staging register
      unsigned int mux_select = _instructions[_pc.read()].mux_select;
      if (mux_select == _num_mem_channels + 1) {
        for (unsigned int e = 0; e < _num_elements_narrow; e++) {
          _staging_data[(_staging_counter * _num_elements_narrow) + e] = 0;
        }
      } else if (mux_select > 0) {
        data_vector<int16_t> popped_data = _input_fifos[mux_select - 1].front();
        for (unsigned int e = 0; e < _num_elements_narrow; e++) {
          _staging_data[(_staging_counter * _num_elements_narrow) + e] =
              popped_data[e];
        }
      }

      if (mux_select > 0) {
        if (_staging_counter == _afifo_width_ratio_out - 1) {
          _staging_counter = 0;
          _output_fifos[_dest_ofifo.read()].push(_staging_data);
          bool padding = true;
          for (unsigned int i = 0; i < _staging_data.size(); i++) {
            if (_staging_data[i] != 0) {
              padding = false;
              break;
            }
          }
          if (!padding) {
            *_debug_feature_interaction_out << _staging_data << "\n";
          }
          _debug_feature_interaction_out->flush();
          if (_dest_ofifo.read() == _num_output_channels - 1) {
            _dest_ofifo.write(0);
          } else {
            _dest_ofifo.write(_dest_ofifo.read() + 1);
          }
        } else {
          _staging_counter++;
        }
      }

      // Pop selected iFIFOs
      for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
        if (_instructions[_pc.read()].fifo_pops[ch_id]) {
          _input_fifos[ch_id].pop();
        }
      }

      // Advance Instructions Pointer
      if (_pc.read() == _instructions.size() - 1) {
        _pc.write(0);
        assert(_staging_counter == 0);
      } else {
        _pc.write(_pc.read() + 1);
      }
    }

    // Interface with AXI-S NoC
    for (unsigned int ch_id = 0; ch_id < _num_output_channels; ch_id++) {
      if (axis_interface[ch_id].tready.read() &&
          axis_interface[ch_id].tvalid.read()) {
        _output_fifos[ch_id].pop();
        // std::cout << "FI sent out vector to MVM " << ch_id << " at cycle "
        //           << GetSimulationCycle(5.0) << std::endl;
      }

      if (!_output_fifos[ch_id].empty()) {
        data_vector<int16_t> tx_tdata = _output_fifos[ch_id].front();
        sc_bv<AXIS_MAX_DATAW> tx_tdata_bv;
        data_vector_to_bv(tx_tdata, tx_tdata_bv, _num_elements_wide_out);
        axis_interface[ch_id].tvalid.write(true);
        axis_interface[ch_id].tdata.write(tx_tdata_bv);
        axis_interface[ch_id].tuser.write(3 << 13);
        axis_interface[ch_id].tid.write(0);
        std::string dest_name =
            "layer0_mvm" + std::to_string(ch_id) + ".rx_interface";
        axis_interface[ch_id].tdest.write(
            radsim_design.GetPortDestinationID(dest_name));
      } else {
        axis_interface[ch_id].tvalid.write(false);
      }
    }

    // Interface with testbench
    /*if (!_ofifo_empty[_src_ofifo.read()] && feature_interaction_ready.read())
    { feature_interaction_valid.write(true);
      feature_interaction_odata.write(_output_fifos[_src_ofifo.read()].front());
      _output_fifos[_src_ofifo.read()].pop();
      if (_src_ofifo.read() == _num_output_channels - 1) {
        _src_ofifo.write(0);
      } else {
        _src_ofifo.write(_src_ofifo.read() + 1);
      }
    } else {
      feature_interaction_valid.write(false);
    }*/

    // Set FIFO signals
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      _ififo_empty[ch_id].write(_input_fifos[ch_id].empty());
      _ififo_full[ch_id].write(_input_fifos[ch_id].size() >=
                               (_fifos_depth - _afifo_width_ratio_in));
    }
    for (unsigned int ch_id = 0; ch_id < _num_output_channels; ch_id++) {
      _ofifo_empty[ch_id].write(_output_fifos[ch_id].empty());
      _ofifo_full[ch_id].write(_output_fifos[ch_id].size() >= _fifos_depth - 2);
    }
    received_responses.write(_num_received_responses);
    /*for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      std::cout << "iFIFO " << ch_id
                << " occupancy = " << _input_fifos[ch_id].size() << std::endl;
    }
    for (unsigned int ch_id = 0; ch_id < _num_output_channels; ch_id++) {
      std::cout << "oFIFO " << ch_id
                << " occupancy = " << _output_fifos[ch_id].size() << std::endl;
    }
    for (unsigned int i = 0; i < _num_mem_channels; i++) {
      std::cout << this->name() << " - " << i << ": " << _input_fifos[i].size()
                << std::endl;
    }
    for (unsigned int i = 0; i < _num_output_channels; i++) {
      std::cout << this->name() << " - " << i << ": " << _output_fifos[i].size()
                << std::endl;
    }*/
    wait();
  }
}

void feature_interaction::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;

  for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
    port_name = module_name + ".aximm_interface_" + std::to_string(ch_id);
    RegisterAximmMasterPort(port_name, &aximm_interface[ch_id], _dataw);
  }

  for (unsigned int ch_id = 0; ch_id < _num_output_channels; ch_id++) {
    port_name = module_name + ".axis_interface_" + std::to_string(ch_id);
    RegisterAxisMasterPort(port_name, &axis_interface[ch_id], DATAW, 0);
  }
}
