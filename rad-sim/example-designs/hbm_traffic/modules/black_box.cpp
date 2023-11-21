#include <black_box.hpp>

// template <typename T>
// void axis_bv_to_data_vector(
//     sc_bv<AXIS_MAX_DATAW> &bitvector, 
//     data_vector<T> &datavector, 
//     unsigned int bitwidth, 
//     unsigned int num_elements) {

//   unsigned int start_idx, end_idx;
//   for (unsigned int e = 0; e < num_elements; e++) {
//     start_idx = e * bitwidth;
//     end_idx = (e + 1) * bitwidth;
//     datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
//   }
// }

// template <typename T>
// void aximm_bv_to_data_vector(
//     sc_bv<AXI4_MAX_DATAW> &bitvector, 
//     data_vector<T> &datavector, 
//     unsigned int bitwidth, 
//     unsigned int num_elements) {

//   unsigned int start_idx, end_idx;
//   for (unsigned int e = 0; e < num_elements; e++) {
//     start_idx = e * bitwidth;
//     end_idx = (e + 1) * bitwidth;
//     datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
//   }
// }

// template <typename T>
// void data_vector_to_axis_bv( 
//     data_vector<T> &datavector, 
//     sc_bv<AXIS_MAX_DATAW> &bitvector, 
//     unsigned int bitwidth, 
//     unsigned int num_elements) {
//     unsigned int start_idx, end_idx;
//     for (unsigned int e = 0; e < num_elements; e++) {
//         start_idx = e * bitwidth;
//         end_idx = (e + 1) * bitwidth;
//         bitvector.range(end_idx - 1, start_idx) = datavector[e];
//     }
// }

// template <typename T>
// void data_vector_to_aximm_bv( 
//     data_vector<T> &datavector, 
//     sc_bv<AXI4_MAX_DATAW> &bitvector, 
//     unsigned int bitwidth, 
//     unsigned int num_elements) {
//     unsigned int start_idx, end_idx;
//     for (unsigned int e = 0; e < num_elements; e++) {
//         start_idx = e * bitwidth;
//         end_idx = (e + 1) * bitwidth;
//         bitvector.range(end_idx - 1, start_idx) = datavector[e];
//     }
// }


black_box::black_box(const sc_module_name &name, 
        unsigned int line_dataw,
        unsigned int element_bitwidth,
        unsigned int fifos_depth) : RADSimModule(name) {

    // _bb_tag = "A";

    _dataw = line_dataw;
    _bitwidth = element_bitwidth;
    _fifos_depth = fifos_depth;

    // init sc vectors
    _mem_req_ififos_full.init(NUM_MEM_REQ_FIFOS);
    _mem_req_ififos_empty.init(NUM_MEM_REQ_FIFOS);


    // _num_mem_channels = num_mem_channels;
    // _num_output_channels = num_output_channels;

    // use the same dataw for input and output bitwidths (can be changed)
    _num_input_elements = line_dataw / element_bitwidth;
    _num_output_elements = line_dataw / element_bitwidth;
    
    // Combinational logic and its sensitivity list
    SC_METHOD(Assign);
    sensitive << rst; 
    sensitive << _rd_data_ififo_full;
    for (int i=0; i < _mem_req_ififos_full.size(); i++) {
        sensitive << _mem_req_ififos_full[i];
    }

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
        // Traffic gen
        mem_req_ready.write(true);

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
        bool all_fifos_not_full = true;

        // Traffic Gen
        for (int i=0; i < _mem_req_ififos_full.size(); i++) {
            all_fifos_not_full = all_fifos_not_full && !_mem_req_ififos_full[i].read();
        }
        mem_req_ready.write(all_fifos_not_full);

        // SISO
        // aximm_interface.bready.write(true); // used to be false but I think I want to it true, write resp channel
        aximm_interface.rready.write(!_rd_data_ififo_full.read()); // for reads we're only ready if the input fifo isn't full
    }
}


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

    // Traffic Gen
    for (int i = 0; i < _mem_req_ififos_full.size(); i++) {
        _mem_req_ififos_full[i].write(false);
        _mem_req_ififos_empty[i].write(true);
    }
    _wr_req_id_count.write(0);
    _wr_data_id_count.write(0);
    _rd_req_id_count.write(0);
    // _rd_data_id_count.write(0);

    // SISO
    aximm_interface.arvalid.write(false);
    aximm_interface.awvalid.write(false);
    aximm_interface.wvalid.write(false);

    // received_responses.write(0);
    // Reset signals
    // SISO
    _rd_data_ififo_full.write(false);
    _rd_data_ififo_empty.write(true);
    _ofifo_full.write(false);
    _ofifo_empty.write(true);


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
        // Traffic Gen
        // if the current mem_req input is valid and the axi is ready to accept a new transaction
        if (mem_req_valid.read() && _target_addr_fifo.size() < _fifos_depth && mem_req_ready.read()) {


            _target_addr_fifo.push(target_address.read());
            _target_channel_fifo.push(target_channel.read());
            _wr_en_fifo.push(wr_en.read());
            _wr_data_fifo.push(wr_data.read());
            _src_port_fifo.push(src_port.read());
            _dst_port_fifo.push(dst_port.read());

            // std::string src_port_name = radsim_design.GetPortNameFromBaseAddress(src_port.read());
            // std::string dst_port_name = radsim_design.GetPortNameFromBaseAddress(dst_port.read());

            if (_wr_en_fifo.front()) {
                std::cout << module_name << ": Received memory write request" /*<< " src: " << src_port_name << " dst: " << dst_port_name*/ << std::endl;
            }
            else {
                std::cout << module_name << ": Received memory read request" /*<< " src: " << src_port_name << " dst: " << dst_port_name*/ << std::endl;
            }

            // std::cout << module_name << ": Received memory request" << std::endl;
        }
        



        // make sure input fifos are not empty
        bool no_mem_req_ififos_empty = std::find_if(_mem_req_ififos_empty.begin(), _mem_req_ififos_empty.end(), [](bool b) {return b == true;}) == _mem_req_ififos_empty.end();
        if (no_mem_req_ififos_empty){
            // Send AXI-MM read/write requests to the NoC        
            uint64_t mem_req_src_port = _src_port_fifo.front();
            uint64_t mem_req_dst_addr = _dst_port_fifo.front() + _target_addr_fifo.front();
            size_t mem_req_wr_data = _wr_data_fifo.front();
            bool mem_req_write_en = _wr_en_fifo.front();
            // unsigned int mem_req_target_channel = _target_channel_fifo.front(); // not using channel at the moment TODO when having single mem ctrl router for multiple channels
            if (mem_req_write_en) {
                // AXI-MM Write
                // Address + Control Channel Ready
                if (aximm_interface.awready.read()) {
                    aximm_interface.awvalid.write(true);
                    aximm_interface.awid.write(_wr_req_id_count.read());
                    aximm_interface.awaddr.write(mem_req_dst_addr);
                    aximm_interface.awlen.write(0); // burst len of 1
                    aximm_interface.awsize.write(0); // burst len of 1
                    aximm_interface.awburst.write(0); // burst len of 1
                    aximm_interface.awuser.write(mem_req_src_port); // assuming the aw user field specifies port where the request came from?
                    // set unused transaction signals to invalid
                    aximm_interface.wvalid.write(false);
                    aximm_interface.arvalid.write(false);

                }
                // Data Channel Ready
                else if (aximm_interface.wready.read()) {
                    aximm_interface.wdata.write(mem_req_wr_data);
                    aximm_interface.wvalid.write(true);
                    aximm_interface.wid.write(_wr_data_id_count.read());
                    aximm_interface.wlast.write(true);
                    aximm_interface.wuser.write(mem_req_src_port); // TODO figure out what the user ports
                    aximm_interface.bready.write(true); // tell AXI that we're ready for mem resp?
                    // set unused transaction signals to invalid
                    aximm_interface.awvalid.write(false);
                    aximm_interface.arvalid.write(false);
                }
                // Write Response Channel , if transaction accepted
                if (aximm_interface.bvalid.read() && aximm_interface.bready.read()) {
                    aximm_interface.bready.write(false);
                    _target_addr_fifo.pop();
                    _target_channel_fifo.pop();
                    _wr_en_fifo.pop();
                    _wr_data_fifo.pop();
                    _src_port_fifo.pop();
                    _dst_port_fifo.pop();
                    // increment transaction ids
                    _wr_data_id_count.write(_wr_data_id_count.read() + 1);
                    _wr_req_id_count.write(_wr_req_id_count.read() + 1);
                }
            } else {
                // AXI-MM Read
                // Address + Control Channel Ready
                aximm_interface.arvalid.write(true);
                aximm_interface.arid.write(_rd_req_id_count.read());
                aximm_interface.araddr.write(mem_req_dst_addr);
                aximm_interface.arlen.write(0); // burst len of 1
                aximm_interface.arsize.write(0); // burst len of 1
                aximm_interface.arburst.write(0); // burst len of 1
                aximm_interface.aruser.write(mem_req_src_port); // assuming the aw user field specifies port where the request came from?
                aximm_interface.rready.write(true);
                
                // If the Address + Control Transaction was recieved
                if (aximm_interface.arvalid.read() && aximm_interface.arready.read()){
                    // pop all fifos (read and write) regardless of if they were valid
                    std::cout << "Sent AR transaction @ id: " << _rd_req_id_count.read() << std::endl;
                    _target_addr_fifo.pop();
                    _target_channel_fifo.pop();
                    _wr_en_fifo.pop();
                    _wr_data_fifo.pop();
                    _src_port_fifo.pop();
                    _dst_port_fifo.pop();
                    _rd_req_id_count.write(_rd_req_id_count.read() + 1);
                }   
            }
        }


        // If transaction accepted and data channel ready
        if (aximm_interface.rvalid.read() && aximm_interface.rready.read()) {
            std::cout << module_name << ": Received READ response "
                        << "@ Cycle " <<  GetSimulationCycle(5.0) << "!"
                        << _num_received_responses << " ("
                        << aximm_interface.rdata.read() << ")!"
                        << std::endl;
        
            // put the returned value from rd transaction into input fifo
            if (_rd_data_input_fifo.size() < _fifos_depth){
                sc_bv<AXI4_MAX_DATAW> rdata_bv = aximm_interface.rdata.read();
                data_vector<int16_t> rdata(_num_input_elements);
                aximm_bv_to_data_vector(rdata_bv, rdata, _bitwidth, _num_input_elements);
                _rd_data_input_fifo.push(rdata);
            }
        }

        // Pop from input FIFOs and push to output FIFOs
        if ( !_rd_data_ififo_empty && !_ofifo_full ) {
            data_vector<int16_t> ofifo_data_vector(_num_output_elements);
            ofifo_data_vector = _rd_data_input_fifo.front();
            _rd_data_input_fifo.pop();
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
            data_vector_to_bv_axis(tx_tdata, tx_tdata_bv, _bitwidth, _num_output_elements);
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

        // Set Instruction FIFO full / empty signals
        for (int i = 0; i < _mem_req_ififos_full.size(); i++) {
            _mem_req_ififos_full[i].write(_target_addr_fifo.size() >= _fifos_depth - 4);
            _mem_req_ififos_empty[i].write(_target_addr_fifo.empty());
        }
        // Set I/O FIFO signals
        // Margin of 4 for input fifos and 2 for outputs not sure why
        _rd_data_ififo_empty.write(_rd_data_input_fifo.empty());
        _rd_data_ififo_full.write(_rd_data_input_fifo.size() >= _fifos_depth - 4); 
        _ofifo_empty.write(_output_fifo.empty());
        _ofifo_full.write(_output_fifo.size() >= _fifos_depth - 2);

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



