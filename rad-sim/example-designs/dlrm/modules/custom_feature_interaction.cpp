#include <custom_feature_interaction.hpp>

void ParseFeatureInteractionInstructions(
    std::string &instructions_file,
    std::vector<custom_feature_interaction_inst> &instructions,
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

  unsigned int fifo_id, start_element, end_element;
  bool pop;
  while (std::getline(inst_file, line)) {
    custom_feature_interaction_inst instruction;
    std::stringstream line_stream(line);
    while (line_stream >> fifo_id >> start_element >> end_element >> pop) {
      instruction.xbar_schedule.push_back(fifo_id);
      instruction.start_element.push_back(start_element);
      instruction.end_element.push_back(end_element);
      instruction.pop_fifo.push_back(pop);
    }
    instructions.push_back(instruction);
  }
  inst_file.close();
}

custom_feature_interaction::custom_feature_interaction(
    const sc_module_name &name, unsigned int dataw,
    unsigned int element_bitwidth, unsigned int num_mem_channels,
    unsigned int fifos_depth, unsigned int num_output_channels,
    std::string &instructions_file,
    RADSimDesignContext* radsim_design)
    : RADSimModule(name, radsim_design) {

  this->radsim_design = radsim_design;
  _fifos_depth = fifos_depth;
  _num_received_responses = 0;
  _num_mem_channels = num_mem_channels;
  _dataw = dataw;
  _bitwidth = element_bitwidth;
  _num_input_elements = dataw / element_bitwidth; //512/16=32
  _num_output_elements = DATAW / element_bitwidth;
  _num_output_channels = num_output_channels;

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

custom_feature_interaction::~custom_feature_interaction() {
  delete _debug_feature_interaction_out;
}

void custom_feature_interaction::Assign() {
  if (rst) {
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      aximm_interface[ch_id].bready.write(false);
      aximm_interface[ch_id].rready.write(false);
    }
  } else if (radsim_design->rad_id == 0) {
    // Set ready signals to accept read/write response from the AXI-MM NoC
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      aximm_interface[ch_id].bready.write(false);
      aximm_interface[ch_id].rready.write(!_ififo_full[ch_id].read());
    }
  }
}

void custom_feature_interaction::bv_to_data_vector(
    sc_bv<AXI4_MAX_DATAW> &bitvector, data_vector<int16_t> &datavector,
    unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * _bitwidth;
    end_idx = (e + 1) * _bitwidth;
    datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
  }
}

void custom_feature_interaction::data_vector_to_bv(
    data_vector<int16_t> &datavector, sc_bv<AXIS_MAX_DATAW> &bitvector,
    unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * _bitwidth;
    end_idx = (e + 1) * _bitwidth;
    bitvector.range(end_idx - 1, start_idx) = datavector[e];
  }
}

bool are_ififos_ready(sc_vector<sc_signal<bool>> &ififo_empty,
                      custom_feature_interaction_inst &inst) {

  bool ready = true;
  for (auto &f : inst.xbar_schedule) {
    if (f == 0)
      ready &= true;
    else
      ready &= !ififo_empty[f - 1].read();
  }
  return ready;
}

void custom_feature_interaction::Tick() {
  if (radsim_design->rad_id == 0) {
  // Reset ports
  for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
    aximm_interface[ch_id].arvalid.write(false);
    aximm_interface[ch_id].awvalid.write(false);
    aximm_interface[ch_id].wvalid.write(false);
  }
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
  _pc.write(0);
  wait();

  int no_val_counter = 0;
  bool got_all_mem_responses = false;

  // Always @ positive edge of the clock
  while (true ) { //&& (radsim_design->rad_id == 0)) {
    // Accept R responses from the NoC
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      if (_input_fifos[ch_id].size() < _fifos_depth &&
          aximm_interface[ch_id].rvalid.read()) {
        sc_bv<AXI4_MAX_DATAW> rdata_bv = aximm_interface[ch_id].rdata.read();
        data_vector<int16_t> rdata(_num_input_elements);
        bv_to_data_vector(rdata_bv, rdata, _num_input_elements);
        _input_fifos[ch_id].push(rdata);
        _num_received_responses++;
        if (_num_received_responses == _num_expected_responses) {
          std::cout << this->name() << ": Got all memory responses at cycle "
                    << GetSimulationCycle(5.0) << "!" << std::endl;
          got_all_mem_responses = true;
        }
      }
    }

    // Pop from input FIFOs
    bool ififos_ready =
        are_ififos_ready(_ififo_empty, _instructions[_pc.read()]);
    if (ififos_ready && !_ofifo_full[_dest_ofifo.read()]) {
      data_vector<int16_t> ofifo_data_vector(_num_output_elements);
      custom_feature_interaction_inst instruction = _instructions[_pc.read()];
      unsigned int num_steps = instruction.xbar_schedule.size();
      unsigned int element_id = 0;
      unsigned int fifo_id, start_idx, end_idx;
      for (unsigned int step = 0; step < num_steps; step++) {
        fifo_id = instruction.xbar_schedule[step];
        start_idx = instruction.start_element[step];
        end_idx = instruction.end_element[step];
        data_vector<int16_t> tmp(_num_input_elements);
        if (fifo_id != 0) {
          tmp = _input_fifos[fifo_id - 1].front();
          if (instruction.pop_fifo[step]) {
            _input_fifos[fifo_id - 1].pop();
          }
        }
        for (unsigned int element = start_idx; element <= end_idx; element++) {
          assert(element_id < ofifo_data_vector.size());
          ofifo_data_vector[element_id] = tmp[element];
          element_id++;
        }
      }
      if (fifo_id != 0) {
        *_debug_feature_interaction_out << ofifo_data_vector << "\n";
        _debug_feature_interaction_out->flush();
      }
      _output_fifos[_dest_ofifo.read()].push(ofifo_data_vector);

      // Advance destination FIFO pointer
      if (_dest_ofifo.read() == _num_output_channels - 1) {
        _dest_ofifo.write(0);
      } else {
        _dest_ofifo.write(_dest_ofifo.read() + 1);
      }

      // Advance Instructions Pointer
      if (_pc.read() == _instructions.size() - 1) {
        _pc.write(0);
      } else {
        _pc.write(_pc.read() + 1);
      }
    }

    // Interface with AXI-S NoC
    bool non_empty_output_fifo = false;
    for (unsigned int ch_id = 0; ch_id < _num_output_channels; ch_id++) {
      if (axis_interface[ch_id].tready.read() &&
          axis_interface[ch_id].tvalid.read()) {
        int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
        data_vector<int16_t> tx_tdata = _output_fifos[ch_id].front();
        //std::cout << "custom_feature_interaction @ cycle " << curr_cycle << ": tx_tdata sent " << tx_tdata << " from RAD " << radsim_design->rad_id << " with tdest field " << axis_interface[ch_id].tdest.read() << std::endl;
        _output_fifos[ch_id].pop();
      }

      if ( (!_output_fifos[ch_id].empty()) ) { //&& (radsim_design->rad_id == 0) ) {
        non_empty_output_fifo = true;
        data_vector<int16_t> tx_tdata = _output_fifos[ch_id].front();
        //std::cout << "custom_feature_interaction: tx_tdata sent " << tx_tdata << " from RAD " << radsim_design->rad_id << std::endl;
        sc_bv<AXIS_MAX_DATAW> tx_tdata_bv;
        data_vector_to_bv(tx_tdata, tx_tdata_bv, _num_output_elements);
        axis_interface[ch_id].tvalid.write(true);
        axis_interface[ch_id].tdata.write(tx_tdata_bv);
        axis_interface[ch_id].tuser.write(3 << 13);
        axis_interface[ch_id].tid.write(0);
        std::string dest_name =
            "layer0_mvm" + std::to_string(ch_id) + ".rx_interface";
        //std::cout << "radsim_design->GetPortDestinationID(dest_name) on RAD " << radsim_design->rad_id << ": " << radsim_design->GetPortDestinationID(dest_name) << std::endl;
        sc_bv<AXIS_DESTW> dest_id_concat;
        DEST_RAD(dest_id_concat) = 1; //radsim_design->rad_id;
        DEST_LOCAL_NODE(dest_id_concat) = radsim_design->GetPortDestinationID(dest_name);
        DEST_REMOTE_NODE(dest_id_concat) = radsim_design->GetPortDestinationID(dest_name);
        axis_interface[ch_id].tdest.write(
            dest_id_concat);
            //radsim_design->GetPortDestinationID(dest_name));
        no_val_counter = 0;
      } else {
        axis_interface[ch_id].tvalid.write(false);
        no_val_counter++;
      }
    }

    // Set FIFO signals
    for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
      _ififo_empty[ch_id].write(_input_fifos[ch_id].empty());
      _ififo_full[ch_id].write(_input_fifos[ch_id].size() >= _fifos_depth - 4);
    }
    for (unsigned int ch_id = 0; ch_id < _num_output_channels; ch_id++) {
      _ofifo_empty[ch_id].write(_output_fifos[ch_id].empty());
      _ofifo_full[ch_id].write(_output_fifos[ch_id].size() >= _fifos_depth - 2);
    }
    received_responses.write(_num_received_responses);

    if (non_empty_output_fifo && got_all_mem_responses) {
      radsim_design->set_rad_done();
    }

    wait();
  }
  }
}

void custom_feature_interaction::RegisterModuleInfo() {
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
