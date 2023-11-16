#include <black_box.hpp>


black_box::black_box(const sc_module_name &name, 
        unsigned int line_dataw,
        unsigned int element_bitwidth,
        unsigned int fifos_depth) : RADSimModule(name) {

    // _bb_tag = "A";

    _dataw = line_dataw;
    _bitwidth = element_bitwidth;
    _fifos_depth = fifos_depth;
    // _num_mem_channels = num_mem_channels;
    // _num_output_channels = num_output_channels;

    // use the same dataw for input and output bitwidths (can be changed)
    _num_input_elements = line_dataw / element_bitwidth;
    _num_output_elements = line_dataw / element_bitwidth;
    
    // Combinational logic and its sensitivity list
    SC_METHOD(Assign);
    sensitive << rst; 
    sensitive << _ififo_full;
    // for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
    //     sensitive << _ififo_full[ch_id];
    // }
    // Sequential logic and its clock/reset setup
    SC_CTHREAD(Tick, clk.pos());
    reset_signal_is(rst, true); // Reset is active high

    // This function must be defined & called for any RAD-Sim module to register
    // its info for automatically connecting to the NoC
    this->RegisterModuleInfo();
    _debug_black_box_out = new ofstream("black_box.out");
}


black_box::~black_box() {
    delete _debug_black_box_out;
}

void black_box::Assign() {
  if (rst) {
    /*
      for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
        // MIMO
        aximm_interface[ch_id].bready.write(false);
        aximm_interface[ch_id].rready.write(false);
        
      }
    */
    // SISO
    aximm_interface.bready.write(false);
    aximm_interface.rready.write(false);
  } else {
    // Set ready signals to accept read/write response from the AXI-MM NoC
    /*
      for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
        // MIMO
        aximm_interface[ch_id].bready.write(false);
        aximm_interface[ch_id].rready.write(!_ififo_full[ch_id].read());
        
      }
    */
    // SISO
    aximm_interface.bready.write(false);
    aximm_interface.rready.write(!_ififo_full.read());
  }
}

template <typename T>
void axis_bv_to_data_vector(
    sc_bv<AXIS_MAX_DATAW> &bitvector, 
    data_vector<T> &datavector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * bitwidth;
    end_idx = (e + 1) * bitwidth;
    datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
  }
}

template <typename T>
void aximm_bv_to_data_vector(
    sc_bv<AXI4_MAX_DATAW> &bitvector, 
    data_vector<T> &datavector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * bitwidth;
    end_idx = (e + 1) * bitwidth;
    datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
  }
}

template <typename T>
void data_vector_to_axis_bv( 
    data_vector<T> &datavector, 
    sc_bv<AXIS_MAX_DATAW> &bitvector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {
    unsigned int start_idx, end_idx;
    for (unsigned int e = 0; e < num_elements; e++) {
        start_idx = e * bitwidth;
        end_idx = (e + 1) * bitwidth;
        bitvector.range(end_idx - 1, start_idx) = datavector[e];
    }
}

template <typename T>
void data_vector_to_aximm_bv( 
    data_vector<T> &datavector, 
    sc_bv<AXI4_MAX_DATAW> &bitvector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {
    unsigned int start_idx, end_idx;
    for (unsigned int e = 0; e < num_elements; e++) {
        start_idx = e * bitwidth;
        end_idx = (e + 1) * bitwidth;
        bitvector.range(end_idx - 1, start_idx) = datavector[e];
    }
}


// void black_box::data_vector_to_bv_aximm(
//     data_vector<int16_t> &datavector, sc_bv<AXI4_MAX_DATAW> &bitvector,
//     unsigned int num_elements) {
//     data_vector_to_bv(datavector, bitvector, num_elements);
// }

// void black_box::data_vector_to_bv_axis(
//     data_vector<int16_t> &datavector, sc_bv<AXIS_MAX_DATAW> &bitvector,
//     unsigned int num_elements) {
//     copy_data_vector_to_bv(datavector, bitvector, num_elements);
// }

// void black_box::bv_to_data_vector(
//     sc_bv<AXI4_MAX_DATAW> &bitvector, data_vector<int16_t> &datavector,
//     unsigned int num_elements) {

//   unsigned int start_idx, end_idx;
//   for (unsigned int e = 0; e < num_elements; e++) {
//     start_idx = e * _bitwidth;
//     end_idx = (e + 1) * _bitwidth;
//     datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
//   }
// }




// void black_box::data_vector_to_bv(
//     data_vector<int16_t> &datavector, sc_bv<AXIS_MAX_DATAW> &bitvector,
//     unsigned int num_elements) {

//   unsigned int start_idx, end_idx;
//   for (unsigned int e = 0; e < num_elements; e++) {
//     start_idx = e * _bitwidth;
//     end_idx = (e + 1) * _bitwidth;
//     bitvector.range(end_idx - 1, start_idx) = datavector[e];
//   }
// }

/*
bool are_ififos_ready(sc_vector<sc_signal<bool>> ififo_empty, uint &pop_fifo_idx) {
    // naive scheduling of memory accesses to fifos, just finds the first fifo available and assigns to it
    bool ready = true;
    for (int i=0; (uint16_t) i < ififo_empty.size(); i++){
        ready &= !ififo_empty[i].read();
        if (ready) {
            pop_fifo_idx = i;
            break;
        }
    }

    return ready;
}
*/


void black_box::Tick() {

    // Reset ports
    // 
    /*
      // MIMO
      for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
          aximm_interface[ch_id].arvalid.write(false);
          aximm_interface[ch_id].awvalid.write(false);
          aximm_interface[ch_id].wvalid.write(false);
      }
    */
    // SISO
    aximm_interface.arvalid.write(false);
    aximm_interface.awvalid.write(false);
    aximm_interface.wvalid.write(false);

    // received_responses.write(0);
    // Reset signals
    // SISO
    _ififo_full.write(false);
    _ififo_empty.write(true);
    _ofifo_full.write(false);
    _ofifo_empty.write(true);

    // FIFOs coming from testbench interface
    /*
      _mem_req_inst_fifo_full.write(false);
      _mem_req_inst_fifo_empty.write(true);
    */

    axis_interface.tvalid.write(false);
    /*
      // MIMO
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
    */
    wait();

    // Always @ positive edge of the clock
    while (true) {

      // Interface with testbench driver
      /*
        if (mem_req_valid.read() && mem_req_ready.read()) {
          data_vector<uint64_t> mem_req_target_address = target_address.read();
          data_vector<unsigned int> mem_req_target_channel = target_channel.read();
          bool mem_req_write_en = write_en.read();
          _target_address_fifo.push(mem_req_target_address);
          _target_channel_fifo.push(mem_req_target_channel);
          _write_en_fifo.push(mem_req_write_en);
          std::cout << module_name << ": Received memory request" << std::endl;
        }
      */
      // Accept R responses from the NoC
      /*
      // MIMO
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
              }
          }
      }
      */
      // if fifo is not full (using SW almost full signal) and aximm return transaction is valid
      if (_input_fifo.size() < _fifos_depth && aximm_interface.rvalid.read() ) {
        sc_bv<AXI4_MAX_DATAW> rdata_bv = aximm_interface.rdata.read();
        data_vector<int16_t> rdata(_num_input_elements);
        aximm_bv_to_data_vector(rdata_bv, rdata, _bitwidth, _num_input_elements);
        _input_fifo.push(rdata);
        _num_received_responses++;
        if (_num_received_responses == _num_expected_responses) {
          std::cout << this->name() << ": Got all memory responses at cycle "
                    << GetSimulationCycle(5.0) << "!" << std::endl;
        }
      }
      // Pop from input FIFOs
      if ( !_ififo_empty && !_ofifo_full ) {
        data_vector<int16_t> ofifo_data_vector(_num_output_elements);
        ofifo_data_vector = _input_fifo.front();
        _input_fifo.pop();
        _output_fifo.push(ofifo_data_vector);
      }

      // Interface with AXI-S NoC
      // if output axi-s tx interface is ready and axi-s tx interface is valid
      if (axis_interface.tready.read() && axis_interface.tvalid.read()){
        _output_fifo.pop();
      }

      if (!_output_fifo.empty()) {
        data_vector<int16_t> tx_tdata = _output_fifo.front();
        sc_bv<AXIS_MAX_DATAW> tx_tdata_bv;
        data_vector_to_axis_bv(tx_tdata, tx_tdata_bv, _bitwidth, _num_output_elements);
        // we init transaction with valid bit, data, user bits, transaction id
        axis_interface.tvalid.write(true);
        axis_interface.tdata.write(tx_tdata_bv);
        axis_interface.tuser.write(3 << 13); // Todo make own tuser spec
        axis_interface.tid.write(0); 
        // Whats the destination of this transaction (AXI-S Slave)
        std::string dest_name = "output_collector.data_collect";
        axis_interface.tdest.write(radsim_design.GetPortDestinationID(dest_name));
      }
      else {
        axis_interface.tvalid.write(false); // if no data in fifo, set valid to false
      }

      // Set I/O FIFO signals
      // Margin of 4 for input fifos and 2 for outputs not sure why
      _ififo_empty.write(_input_fifo.empty());
      _ififo_full.write(_input_fifo.size() >= _fifos_depth - 4); 
      _ofifo_empty.write(_output_fifo.empty());
      _ofifo_full.write(_output_fifo.size() >= _fifos_depth - 2);

      // Set mem req FIFO signals 
      // WARNING these are only looking at the target address fifo, other mem_req fifos are assumed to be synced up
      /*
        _mem_req_inst_fifo_empty.write(_target_address_fifo.empty());
        _mem_req_inst_fifo_full.write(_target_address_fifo.size() >= _fifos_depth - 4); // TODO figure out why Andrew did -4 for the input fifos
      */

      // received_responses.write(_num_received_responses);
      wait();

        /*
        // // Pop from input FIFOs
        // uint pop_ififo_id;
        // bool ififos_ready = are_ififos_ready(_ififo_empty, pop_ififo_id);
        if (ififos_ready && !_ofifo_full[_dest_ofifo.read()]) {
            data_vector<int16_t> ofifo_data_vector(_num_output_elements);

            // custom_feature_interaction_inst instruction = _instructions[_pc.read()];
            // unsigned int num_steps = instruction.xbar_schedule.size();
            unsigned int element_id = 0;
            unsigned int fifo_id, start_idx, end_idx;
            
            ofifo_data_vector = _input_fifos[pop_ififo_id].front();
            _input_fifos[pop_ififo_id].pop();
            _output_fifos[_dest_ofifo.read()].push(ofifo_data_vector);

            // data_vector<int16_t> tmp(_num_input_elements);
            // tmp = _input_fifos[pop_ififo_id].front();
            // _input_fifos[pop_ififo_id].pop();
            

            // for (unsigned int step = 0; step < num_steps; step++) {
            //     // fifo_id = instruction.xbar_schedule[step];
            //     // start_idx = instruction.start_element[step];
            //     // end_idx = instruction.end_element[step];


            //     // if (pop_ififo_id != 0) {
            //     //     tmp = _input_fifos[pop_ififo_id - 1].front();
            //     //     if (instruction.pop_fifo[step]) {
            //     //         _input_fifos[fifo_id - 1].pop();
            //     //     }
            //     // }
            //     for (unsigned int element = start_idx; element <= end_idx; element++) {
            //         assert(element_id < ofifo_data_vector.size());
            //         ofifo_data_vector[element_id] = tmp[element];
            //         element_id++;
            //     }
            // }
            // if (fifo_id != 0) {
            //     *_debug_feature_interaction_out << ofifo_data_vector << "\n";
            //     _debug_feature_interaction_out->flush();
            // }
            // _output_fifos[_dest_ofifo.read()].push(ofifo_data_vector);

            // Advance destination FIFO pointer
            // if (_dest_ofifo.read() == _num_output_channels - 1) {
            //     _dest_ofifo.write(0);
            // } else {
            //     _dest_ofifo.write(_dest_ofifo.read() + 1);
            // }

            // Advance Instructions Pointer
            // if (_pc.read() == _instructions.size() - 1) {
            //     _pc.write(0);
            // } else {
            //     _pc.write(_pc.read() + 1);
            // }

                // Interface with AXI-S NoC
                for (unsigned int ch_id = 0; ch_id < _num_output_channels; ch_id++) {
                    if (axis_interface[ch_id].tready.read() &&
                        axis_interface[ch_id].tvalid.read()) {
                        _output_fifos[ch_id].pop();
                    }

                    if (!_output_fifos[ch_id].empty()) {
                        data_vector<int16_t> tx_tdata = _output_fifos[ch_id].front();
                        sc_bv<AXIS_MAX_DATAW> tx_tdata_bv;
                        data_vector_to_bv(tx_tdata, tx_tdata_bv, _num_output_elements);
                        axis_interface[ch_id].tvalid.write(true);
                        axis_interface[ch_id].tdata.write(tx_tdata_bv);
                        axis_interface[ch_id].tuser.write(3 << 13);
                        axis_interface[ch_id].tid.write(0);
                        // change destination to output collector
                        std::string dest_name =
                            "output_collector.data_collect";
                            // "layer0_mvm" + std::to_string(ch_id) + ".rx_interface";
                        axis_interface[ch_id].tdest.write(
                            radsim_design.GetPortDestinationID(dest_name));
                    } else {
                        axis_interface[ch_id].tvalid.write(false);
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
            wait();
        }
        */

    }
}

void black_box::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;

  port_name = module_name + ".aximm_interface";
  RegisterAximmMasterPort(port_name, &aximm_interface, _dataw);

  port_name = module_name + ".axis_interface";
  RegisterAxisMasterPort(port_name, &axis_interface, _dataw, 0);

  // for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
  //   port_name = module_name + ".aximm_interface_" + std::to_string(ch_id);
  //   RegisterAximmMasterPort(port_name, &aximm_interface[ch_id], _dataw);
  // }

  // for (unsigned int ch_id = 0; ch_id < _num_output_channels; ch_id++) {
  //   port_name = module_name + ".axis_interface_" + std::to_string(ch_id);
  //   RegisterAxisMasterPort(port_name, &axis_interface[ch_id], DATAW, 0);
  // }
}



