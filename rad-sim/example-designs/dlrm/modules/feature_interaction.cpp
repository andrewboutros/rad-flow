#include <feature_interaction.hpp>

void ParseFeatureInteractionInstructions(
    std::string &instructions_file,
    std::vector<feature_interaction_inst> &instructions) {

  std::ifstream inst_file(instructions_file);
  if (!inst_file) {
    sim_log.log(error, "Cannot find feature interaction instructions file!");
  }

  std::string line;
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
}

feature_interaction::feature_interaction(const sc_module_name &name,
                                         unsigned int dataw,
                                         unsigned int element_bitwidth,
                                         unsigned int num_mem_channels,
                                         unsigned int fifos_depth,
                                         std::string &instructions_file)
    : radsim_module(name) {

  _fifos_depth = fifos_depth;
  _num_received_responses = 0;
  _num_mem_channels = num_mem_channels;
  _dataw = dataw;
  _bitwidth = element_bitwidth;
  _num_elements = dataw / element_bitwidth;

  aximm_interface.init(_num_mem_channels);

  std::string module_name_str;
  char module_name[25];

  _input_fifos.resize(_num_mem_channels);
  _ififo_wdata.init(_num_mem_channels);
  _ififo_rdata.init(_num_mem_channels);
  _ififo_wen.init(_num_mem_channels);
  _ififo_ren.init(_num_mem_channels);
  _ififo_full.init(_num_mem_channels);
  _ififo_afull.init(_num_mem_channels);
  _ififo_empty.init(_num_mem_channels);
  for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
    module_name_str = "ififo_" + std::to_string(ch_id);
    std::strcpy(module_name, module_name_str.c_str());
    _input_fifos[ch_id] = new afifo<int16_t>(module_name, _fifos_depth,
                                             _dataw / 16, 4, _fifos_depth - 4);
    _input_fifos[ch_id]->clk(clk);
    _input_fifos[ch_id]->rst(rst);
    _input_fifos[ch_id]->wen(_ififo_wen[ch_id]);
    _input_fifos[ch_id]->wdata(_ififo_wdata[ch_id]);
    _input_fifos[ch_id]->ren(_ififo_ren[ch_id]);
    _input_fifos[ch_id]->rdata(_ififo_rdata[ch_id]);
    _input_fifos[ch_id]->full(_ififo_full[ch_id]);
    _input_fifos[ch_id]->almost_full(_ififo_afull[ch_id]);
    _input_fifos[ch_id]->empty(_ififo_empty[ch_id]);
  }

  module_name_str = "ofifo";
  std::strcpy(module_name, module_name_str.c_str());
  _output_fifo = new afifo<int16_t>(module_name, _fifos_depth, 4, _dataw / 16,
                                    _fifos_depth - 4);
  _output_fifo->clk(clk);
  _output_fifo->rst(rst);
  _output_fifo->wen(_ofifo_wen);
  _output_fifo->wdata(_ofifo_wdata);
  _output_fifo->ren(_ofifo_ren);
  _output_fifo->rdata(_ofifo_rdata);
  _output_fifo->full(_ofifo_full);
  _output_fifo->almost_full(_ofifo_afull);
  _output_fifo->empty(_ofifo_empty);

  ParseFeatureInteractionInstructions(instructions_file, _instructions);

  // Combinational logic and its sensitivity list
  SC_METHOD(Assign);
  sensitive << rst << _ofifo_empty << _ofifo_rdata << feature_interaction_valid
            << feature_interaction_ready;
  for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
    sensitive << _ififo_full[ch_id];
  }
  // Combinational logic and its sensitivity list
  SC_METHOD(AssignPC);
  sensitive << rst << _pc << _ofifo_afull;
  for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
    sensitive << _ififo_empty[ch_id] << _ififo_rdata[ch_id];
  }
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

feature_interaction::~feature_interaction() {
  for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
    delete _input_fifos[ch_id];
  }
  delete _output_fifo;
}

void feature_interaction::Assign() {
  if (rst) {
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      aximm_interface[ch_id].bready.write(false);
      aximm_interface[ch_id].rready.write(false);
    }
    feature_interaction_valid.write(false);
    _ofifo_ren.write(false);
  } else {
    // Always ready to accept read/write response from the AXI-MM NoC
    // interface
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      aximm_interface[ch_id].bready.write(false);
      aximm_interface[ch_id].rready.write(!_ififo_full[ch_id].read());
    }
    feature_interaction_valid.write(!_ofifo_empty.read());
    feature_interaction_odata.write(_ofifo_rdata.read());
    _ofifo_ren.write(feature_interaction_valid.read() &&
                     feature_interaction_ready.read());
  }
}

void feature_interaction::AssignPC() {
  if (rst) {
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      _ififo_ren[ch_id].write(false);
    }
    _ofifo_wen.write(false);
  } else {
    // Check that all FIFOs to be popped are not empty
    bool ififos_rd_ok = true;
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      if (_instructions[_pc.read()].fifo_pops[ch_id]) {
        ififos_rd_ok = ififos_rd_ok && !_ififo_empty[ch_id].read();
      }
    }

    // If iFIFOs are ok, pop all & push one (or padding) to the oFIFO
    if (ififos_rd_ok && !_ofifo_afull.read()) {
      for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
        _ififo_ren[ch_id].write(_instructions[_pc.read()].fifo_pops[ch_id]);
      }

      unsigned int mux_select = _instructions[_pc.read()].mux_select;
      if (mux_select == _num_mem_channels + 1) {
        data_vector<int16_t> all_zeros(_num_elements);
        _ofifo_wdata.write(all_zeros);
        _ofifo_wen.write(true);
      } else if (mux_select > 0) {
        _ofifo_wdata.write(_ififo_rdata[mux_select - 1].read());
        _ofifo_wen.write(true);
        data_vector<int16_t> rdata = _ififo_rdata[mux_select - 1].read();
        // std::cout << GetSimulationCycle(5.0) << ": Steer iFIFO "
        //           << mux_select - 1 << " to oFIFO (PC = " << _pc.read()
        //           << "): " << rdata << std::endl;
      } else {
        _ofifo_wen.write(false);
      }

    } else {
      for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
        _ififo_ren[ch_id].write(false);
      }
      _ofifo_wen.write(false);
    }
  }
}

void feature_interaction::bv_to_data_vector(sc_bv<AXI_MAX_DATAW> &bitvector,
                                            data_vector<int16_t> &datavector) {

  unsigned int start_idx, end_idx;
  for (unsigned int element_id = 0; element_id < _num_elements; element_id++) {
    start_idx = element_id * _bitwidth;
    end_idx = (element_id + 1) * _bitwidth;
    datavector[element_id] = bitvector.range(end_idx - 1, start_idx).to_int();
  }
}

void feature_interaction::Tick() {
  // Reset logic
  for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
    aximm_interface[ch_id].arvalid.write(false);
    aximm_interface[ch_id].awvalid.write(false);
    aximm_interface[ch_id].wvalid.write(false);
    _ififo_wen[ch_id].write(false);
  }
  received_responses.write(0);
  wait();

  // Always @ positive edge of the clock
  while (true) {
    // Receiving transactions from AXI-MM NoC
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      if (aximm_interface[ch_id].rvalid.read() &&
          aximm_interface[ch_id].rready.read()) {
        data_vector<int16_t> rdata(_num_elements);
        sc_bv<AXI_MAX_DATAW> bitvector = aximm_interface[ch_id].rdata.read();
        bv_to_data_vector(bitvector, rdata);
        _ififo_wdata[ch_id].write(rdata);
        _ififo_wen[ch_id].write(true);
        // std::cout << "IFIFO " << ch_id << " PUSH: " << rdata << std::endl;
        // std::cout << module_name << ": Pushed read response to input FIFO "
        //            << ch_id << " [" << rdata << "]" << std::endl;
        _num_received_responses++;
      } else if (aximm_interface[ch_id].bvalid.read() &&
                 aximm_interface[ch_id].bready.read()) {
        _num_received_responses++;
        _ififo_wen[ch_id].write(false);
      } else {
        _ififo_wen[ch_id].write(false);
      }
    }

    // Check that all FIFOs to be popped are not empty
    bool ififos_rd_ok = true;
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      if (_instructions[_pc.read()].fifo_pops[ch_id]) {
        ififos_rd_ok = ififos_rd_ok && !_ififo_empty[ch_id].read();
      }
    }

    // If iFIFOs are ok, pop all & push one (or padding) to the oFIFO
    if (ififos_rd_ok && !_ofifo_afull.read()) {
      if (_pc.read() == _instructions.size() - 1) {
        _pc.write(0);
      } else {
        _pc.write(_pc.read() + 1);
      }
    }

    received_responses.write(_num_received_responses);
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
}
