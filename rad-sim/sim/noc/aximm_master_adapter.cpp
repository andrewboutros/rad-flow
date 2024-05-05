#include "aximm_master_adapter.hpp"

aximm_master_adapter::aximm_master_adapter(
    const sc_module_name &name, int node_id, int network_id,
    BookSimConfig *noc_config, Network *noc, BufferState *buffer_state,
    tRoutingFunction routing_func, bool lookahead_routing,
    bool wait_for_tail_credit, map<int, int> *ejected_flits,
    unsigned int interface_dataw, double node_period, double adapter_period,
    RADSimConfig* radsim_config //AKB added
    )
    : sc_module(name) {

  //AKB ADDED
  this->radsim_config = radsim_config;

  // Initialize basic adapter member variables
  _node_id = node_id;
  _network_id = network_id;
  _node_period = node_period;
  _adapter_period = adapter_period;
  _noc_period = radsim_config->GetDoubleVectorKnob("noc_clk_period", _network_id);
  _interface_dataw = interface_dataw;

  _noc_config = noc_config;
  _noc = noc;
  _buffer_state = buffer_state;
  _routing_func = routing_func;
  _lookahead_routing = lookahead_routing;
  _wait_for_tail_credit = wait_for_tail_credit;
  _ejected_flits = ejected_flits;

  // Initialize request interface (AR, AW, W) member variables
  _ejection_afifo_depth =
      radsim_config->GetIntVectorKnob("noc_adapters_fifo_size", _network_id);
  _ejection_afifos.resize(AXI_NUM_REQ_TYPES);
  _ejection_afifo_push_counter.init(AXI_NUM_REQ_TYPES);
  _ejection_afifo_pop_counter.init(AXI_NUM_REQ_TYPES);
  for (int i = 0; i < AXI_NUM_REQ_TYPES; i++) {
    _ejection_afifo_priority.push(i);
  }
  _ejected_booksim_flit = nullptr;

  // Request pipeline modules described as SystemC methods and threads
  SC_CTHREAD(OutputEjection, noc_clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(OutputDepacketization, adapter_clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(OutputInterface, node_clk.pos());
  reset_signal_is(rst, true);

  // Initialize response interface (B, R) member variables
  _injection_afifo_depth =
      radsim_config->GetIntVectorKnob("noc_adapters_fifo_size", _network_id);
  _axi_transaction_width = AXI4_USERW;
  if ((AXI4_ADDRW + AXI4_CTRLW) > (_interface_dataw + AXI4_RESPW + 1)) {
    _axi_transaction_width += (AXI4_ADDRW + AXI4_CTRLW);
  } else {
    _axi_transaction_width += (_interface_dataw + AXI4_RESPW + 1);
  }
  _max_flits_per_transaction =
      (int)ceil(_axi_transaction_width / NOC_LINKS_PAYLOAD_WIDTH);
  _freq_ratio = (int)floor(_node_period / _adapter_period);
  _rready = false;
  _bready = false;
  _injection_flit_ready = false;
  _output_packet_ready = false;
  _last_vc_id = 0;

  // Response pipeline modules described as SystemC methods and threads
  SC_METHOD(InputReady);
  sensitive << _injection_afifo_full << aximm_interface.rvalid
            << aximm_interface.bvalid << _injection_priority_setting
            << _packetization_cycle << rst;
  SC_CTHREAD(InputInjection, noc_clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(InputPacketization, adapter_clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(InputInterface, node_clk.pos());
  reset_signal_is(rst, true);
}

aximm_master_adapter::~aximm_master_adapter() {
  delete _noc;
  delete _noc_config;
}

int aximm_master_adapter::InverseVCMapping(int vc_id) {
  // Get the start and end VCs of AW, AR and W transactions from Booksim NoC
  // configuration
  int aw_vc_start = _noc_config->GetInt("write_request_begin_vc");
  int aw_vc_end = _noc_config->GetInt("write_request_end_vc");
  int ar_vc_start = _noc_config->GetInt("read_request_begin_vc");
  int ar_vc_end = _noc_config->GetInt("read_request_end_vc");
  int w_vc_start = _noc_config->GetInt("write_data_begin_vc");
  int w_vc_end = _noc_config->GetInt("write_data_end_vc");

  // Categorize the received flit based on its VC ID: 0 for R transaction, 1 for
  // B transaction
  if (vc_id >= aw_vc_start && vc_id <= aw_vc_end) {
    return AXI_TYPE_AW;
  } else if (vc_id >= ar_vc_start && vc_id <= ar_vc_end) {
    return AXI_TYPE_AR;
  } else if (vc_id >= w_vc_start && vc_id <= w_vc_end) {
    return AXI_TYPE_W;
  } else {
    cerr << this->name() << " Request VC ID (" << vc_id
         << ") does not correspond to a Request VC" << endl;
    exit(1);
  }
}

void aximm_master_adapter::OutputEjection() {
  // Reset Code
  for (auto &ejection_fifo_push_counter : _ejection_afifo_push_counter) {
    ejection_fifo_push_counter.write(0);
  }
  wait();

  while (true) {
    // If previously ejected flit was pused to a fifo, try read a new one
    if (_ejected_booksim_flit == nullptr) {
      _ejected_booksim_flit = _noc->ReadFlit(_node_id);
    }

    // If reading a flit from the NoC is successful, start processing it
    if (_ejected_booksim_flit) {
      // Check if corresponding ejection FIFO can accept a new flit
      int vc_id = _ejected_booksim_flit->vc;
      // std::cout << this->name() << ": Ejected Flit from VC " << vc_id << "!"
      //           << std::endl;
      int type = InverseVCMapping(vc_id);
      int type_id = (type == AXI_TYPE_AR) ? 0 : (type == AXI_TYPE_AW) ? 1 : 2;
      if (_ejection_afifos[type_id].size() < _ejection_afifo_depth) {
        // Create a SystemC flit and push it to its corresponding ejection FIFO
        sc_flit ejected_flit(
            _ejected_booksim_flit->head, _ejected_booksim_flit->tail,
            _ejected_booksim_flit->type, _ejected_booksim_flit->vc,
            _ejected_booksim_flit->dest, _ejected_booksim_flit->dest_interface,
            _ejected_booksim_flit->pid, _ejected_booksim_flit->id);
        ejected_flit._payload = static_cast<sc_bv<NOC_LINKS_PAYLOAD_WIDTH> *>(
            _ejected_booksim_flit->data);
        _ejection_afifos[type_id].push(ejected_flit);

        // If tail flit received, increment FIFO counter (i.e. FIFO has 1 more
        // complete packet)
        if (_ejected_booksim_flit->tail) {
          int updated_count = _ejection_afifo_push_counter[type_id].read() + 1;
          _ejection_afifo_push_counter[type_id].write(updated_count);
          NoCTransactionTelemetry::RecordTransactionTailEjection(
              _ejected_booksim_flit->id);
          NoCTransactionTelemetry::UpdateHops(_ejected_booksim_flit->id,
                                              _ejected_booksim_flit->hops);
        }

        // Log flit receipt and set _ejected_booksim_flit to NULL to indicate it
        // was processed
        _ejected_booksim_flit = nullptr;
      }
    }
    wait();
  }
}

void aximm_master_adapter::OutputDepacketization() {
  // Reset Code
  for (auto &ejection_afifo_pop_counter : _ejection_afifo_pop_counter) {
    ejection_afifo_pop_counter.write(0);
  }
  _ejection_afifo_is_depacketizing.write(false);

  while (true) {
    unsigned int highest_priority_fifo_id = _ejection_afifo_priority.front();
    bool fifo_has_complete_packet =
        _ejection_afifo_push_counter[highest_priority_fifo_id].read() >
        _ejection_afifo_pop_counter[highest_priority_fifo_id].read();

    // If no ejection FIFO is currently being depacketized
    if (!_ejection_afifo_is_depacketizing.read()) {

      // Loop over ejection FIFOs based on current priority status
      // By the end of this while loop: If fifo_has_complete_packet is true, the
      // highest priority FIFO index will be at the top of the priority queue.
      // If fifo_has_complete_packet is false, no FIFO has a complete packet
      // ready
      unsigned int num_checked_fifos = 0;
      while (!fifo_has_complete_packet &&
             num_checked_fifos < AXI_NUM_REQ_TYPES) {
        _ejection_afifo_priority.push(_ejection_afifo_priority.front());
        _ejection_afifo_priority.pop();
        num_checked_fifos++;
        highest_priority_fifo_id = _ejection_afifo_priority.front();
        fifo_has_complete_packet =
            _ejection_afifo_push_counter[highest_priority_fifo_id].read() >
            _ejection_afifo_pop_counter[highest_priority_fifo_id].read();
      }
    }

    // If there is a complete packet, set depacketization flag and start
    // processing it
    if (fifo_has_complete_packet) {
      // If output double buffer (FIFO of size 2) has free space, depacketize
      // flits
      if (_output_afifo.size() < 2) {
        // Pop a flit from the highest priority ejection FIFO
        sc_flit processed_flit =
            _ejection_afifos[highest_priority_fifo_id].front();
        _ejection_afifos[highest_priority_fifo_id].pop();
        _constructed_packet.AddFlit(processed_flit);

        // Write credit for the accepted flit to the corresponding VC
        _ejected_flits->insert(
            make_pair(_node_id, processed_flit._vc_id.to_uint()));

        // If the flit is a tail, push constructed packet to output double
        // buffer & reset depacketization flag
        if (processed_flit._tail) {
          _output_afifo.push(_constructed_packet);
          NoCTransactionTelemetry::RecordTransactionTailDepacketization(
              processed_flit._sim_transaction_id);
          _constructed_packet.Reset();
          _ejection_afifo_is_depacketizing.write(false);

          // Update FIFO packet pop counter
          int updated_count =
              _ejection_afifo_pop_counter[highest_priority_fifo_id].read() + 1;
          _ejection_afifo_pop_counter[highest_priority_fifo_id].write(
              updated_count);
          assert(updated_count <=
                 _ejection_afifo_push_counter[highest_priority_fifo_id]);

          // Adjust ejection FIFOs priority status
          _ejection_afifo_priority.push(_ejection_afifo_priority.front());
          _ejection_afifo_priority.pop();
          // std::cout << this->name() << ": Depacketized packet!" << std::endl;
        } else {
          _ejection_afifo_is_depacketizing.write(true);
        }
      } else {
        _ejection_afifo_is_depacketizing.write(true);
      }
    }
    wait();
  }
}

void aximm_master_adapter::OutputInterface() {
  // Reset
  aximm_interface.arvalid.write(false);
  aximm_interface.awvalid.write(false);
  aximm_interface.wvalid.write(false);
  _output_packet_ready = false;
  wait();

  while (true) {
    if (!_output_afifo.empty() > 0 && !_output_packet_ready) {
      _output_packet = _output_afifo.front();
      _output_afifo.pop();
      _output_packet_ready = true;
    }
    if (_output_packet_ready &&
        (_output_packet.GetFlit(0)->_type == Flit::FlitType::READ_REQUEST) &&
        aximm_interface.arready.read()) {

      sc_bv<AXI4_PAYLOADW> temp_bv;
      sc_flit *temp_flit = _output_packet.GetFlit(0);

      for (unsigned int i = 0; i < _output_packet.GetNumValidFlits(); i++) {
        unsigned int start_idx = i * NOC_LINKS_PAYLOAD_WIDTH;
        unsigned int end_idx =
            min((int)AXI4_PAYLOADW, (int)(i + 1) * NOC_LINKS_PAYLOAD_WIDTH);
        sc_bv<NOC_LINKS_PAYLOAD_WIDTH> flit_payload =
            *(_output_packet.GetFlit(i)->_payload);
        temp_bv.range(end_idx - 1, start_idx) = flit_payload;
      }

      aximm_interface.awvalid.write(false);
      aximm_interface.wvalid.write(false);
      aximm_interface.arvalid.write(true);
      aximm_interface.arid.write(temp_flit->_packet_id);
      aximm_interface.araddr.write(AXI4_ADDR(temp_bv));
      aximm_interface.aruser.write(AXI4_USER(temp_bv));
      sc_bv<AXI4_CTRLW> ctrl_temp = AXI4_CTRL(temp_bv);
      unsigned int offset = 0;
      aximm_interface.arburst.write(
          temp_bv.range(offset + AXI4_BURSTW - 1, offset).to_uint());
      offset += AXI4_BURSTW;
      aximm_interface.arsize.write(
          temp_bv.range(offset + AXI4_SIZEW - 1, offset).to_uint());
      offset += AXI4_SIZEW;
      aximm_interface.arlen.write(
          temp_bv.range(offset + AXI4_LENW - 1, offset).to_uint());
      _output_packet_ready = false;
      NoCTransactionTelemetry::RecordTransactionReceipt(
          temp_flit->_sim_transaction_id);
      // std::cout << this->name() << ": Got AR request!" << std::endl;
    } else if (_output_packet_ready &&
               (_output_packet.GetFlit(0)->_type ==
                Flit::FlitType::WRITE_REQUEST) &&
               aximm_interface.awready.read()) {

      sc_bv<AXI4_PAYLOADW> temp_bv;
      sc_flit *temp_flit = _output_packet.GetFlit(0);

      for (unsigned int i = 0; i < _output_packet.GetNumValidFlits(); i++) {
        unsigned int start_idx = i * NOC_LINKS_PAYLOAD_WIDTH;
        unsigned int end_idx =
            min((int)AXI4_PAYLOADW, (int)(i + 1) * NOC_LINKS_PAYLOAD_WIDTH);
        sc_bv<NOC_LINKS_PAYLOAD_WIDTH> flit_payload =
            *(_output_packet.GetFlit(i)->_payload);
        temp_bv.range(end_idx - 1, start_idx) = flit_payload;
      }

      aximm_interface.arvalid.write(false);
      aximm_interface.wvalid.write(false);
      aximm_interface.awvalid.write(true);
      aximm_interface.awid.write(temp_flit->_packet_id);
      aximm_interface.awaddr.write(AXI4_ADDR(temp_bv));
      aximm_interface.awuser.write(AXI4_USER(temp_bv));
      sc_bv<AXI4_CTRLW> ctrl_temp = AXI4_CTRL(temp_bv);
      unsigned int offset = 0;
      aximm_interface.awburst.write(
          temp_bv.range(offset + AXI4_BURSTW - 1, offset).to_uint());
      offset += AXI4_BURSTW;
      aximm_interface.awsize.write(
          temp_bv.range(offset + AXI4_SIZEW - 1, offset).to_uint());
      offset += AXI4_SIZEW;
      aximm_interface.awlen.write(
          temp_bv.range(offset + AXI4_LENW - 1, offset).to_uint());
      _output_packet_ready = false;
      NoCTransactionTelemetry::RecordTransactionReceipt(
          temp_flit->_sim_transaction_id);
    } else if (_output_packet_ready &&
               (_output_packet.GetFlit(0)->_type ==
                Flit::FlitType::WRITE_DATA) &&
               aximm_interface.wready.read()) {

      sc_bv<AXI4_PAYLOADW> temp_bv;
      sc_flit *temp_flit = _output_packet.GetFlit(0);

      for (unsigned int i = 0; i < _output_packet.GetNumValidFlits(); i++) {
        unsigned int start_idx = i * NOC_LINKS_PAYLOAD_WIDTH;
        unsigned int end_idx =
            min((int)AXI4_PAYLOADW, (int)(i + 1) * NOC_LINKS_PAYLOAD_WIDTH);
        sc_bv<NOC_LINKS_PAYLOAD_WIDTH> flit_payload =
            *(_output_packet.GetFlit(i)->_payload);
        temp_bv.range(end_idx - 1, start_idx) = flit_payload;
      }

      aximm_interface.arvalid.write(false);
      aximm_interface.awvalid.write(false);
      aximm_interface.wvalid.write(true);
      aximm_interface.wid.write(temp_flit->_packet_id);
      aximm_interface.wlast.write(AXI4_LAST(temp_bv).to_uint());
      aximm_interface.wdata.write(AXI4_DATA(temp_bv));
      aximm_interface.wuser.write(AXI4_USER(temp_bv));
      _output_packet_ready = false;
      NoCTransactionTelemetry::RecordTransactionReceipt(
          temp_flit->_sim_transaction_id);
    } else {
      aximm_interface.arvalid.write(false);
      aximm_interface.awvalid.write(false);
      aximm_interface.wvalid.write(false);
    }
    wait();
  }
}

// Implements the input ready interface based on prioritized round-robin
// arbitration. This decides which interface (R, B) is captured and passed to
// the packetization stage.
void aximm_master_adapter::InputReady() {
  if (rst.read()) {
    _rready = false;
    _bready = false;
  } else {
    // If priority setting is 0, check R->B. If priority setting is 1, check
    // B->R. The priority setting is changed as soon as a transaction is
    // selected and captured.
    if (_injection_priority_setting.read() == 0) {
      _rready = !_injection_afifo_full && aximm_interface.rvalid.read();
      _bready =
          !_injection_afifo_full && !_rready && aximm_interface.bvalid.read();
    } else {
      _bready = !_injection_afifo_full && aximm_interface.bvalid.read();
      _rready =
          !_injection_afifo_full && !_bready && aximm_interface.rvalid.read();
    }

    // Verify that the rready and bready signals are never asserted
    // simultaneously.
    assert(!(_rready && _bready));
  }
  aximm_interface.rready.write((_packetization_cycle.read() == 0) && _rready);
  aximm_interface.bready.write((_packetization_cycle.read() == 0) && _bready);
}

int aximm_master_adapter::GetInputDestinationNode(sc_bv<AXI4_ADDRW> &addr) {
  return addr.range(AXI4_ADDRW - 1, AXI4_ADDRW - AXI4_NODE_ADDRW)
      .to_uint();
}

// Implements the input interface capturing into the reduced set of signals to
// be bassed to the adapter packetization stage based on the set valid & ready
// signals.
void aximm_master_adapter::InputInterface() {
  // Reset
  _i_id.write(0);
  _i_payload.write(0);
  _i_ctrl.write(0);
  _i_user.write(0);
  _i_valid.write(false);
  _i_type.write(0);
  _i_noc_dest.write(0);
  _i_resp.write(0);
  _i_last.write(0);
  _i_unique_sim_id.write(0);
  _injection_priority_setting.write(0);
  wait();

  // Functionality
  while (true) {
    if (aximm_interface.rready.read() && aximm_interface.rvalid.read()) {
      // Capture the values at the AXI R port
      _i_id.write(aximm_interface.rid.read());
      _i_payload.write(aximm_interface.rdata.read());
      _i_resp.write(aximm_interface.rresp.read());
      _i_last.write(aximm_interface.rlast.read());
      _i_user.write(aximm_interface.ruser.read());
      _i_valid.write(aximm_interface.rvalid.read());
      _i_type.write(AXI_TYPE_R);
      sc_bv<AXI4_ADDRW> resp_addr = aximm_interface.ruser.read().to_uint64();
      int noc_dest = GetInputDestinationNode(resp_addr);
      _i_noc_dest.write(noc_dest);
      // std::cout << this->name()
      //           << ": Registered R response with destination node " <<
      //           noc_dest
      //           << std::endl;

      // Adjust priority setting and log the initiation of an R transaction
      _injection_priority_setting.write(1);

      // Record initiation time in simulation statistics
      int unique_sim_id = NoCTransactionTelemetry::RecordTransactionInitiation(
          _node_id, noc_dest, AXI_TYPE_R, _interface_dataw, _network_id);
      _i_unique_sim_id.write(unique_sim_id);
    } else if (aximm_interface.bready.read() && aximm_interface.bvalid.read()) {
      // Capture the values at the AXI B port
      _i_id.write(aximm_interface.bid.read());
      _i_resp.write(aximm_interface.bresp.read());
      _i_user.write(aximm_interface.buser.read());
      _i_valid.write(aximm_interface.bvalid.read());
      _i_type.write(AXI_TYPE_B);
      sc_bv<AXI4_ADDRW> resp_addr = aximm_interface.buser.read().to_uint64();
      int noc_dest = GetInputDestinationNode(resp_addr);
      _i_noc_dest.write(noc_dest);
      /*std::cout << this->name()
                << ": Registered B transaction with destination node "
                << noc_dest << " and resp address " << resp_addr.to_uint64()
                << "!" << std::endl;*/

      // Adjust priority setting and log the initiation of a B transaction
      _injection_priority_setting.write(0);

      // Record initiation time in simulation statistics
      int unique_sim_id = NoCTransactionTelemetry::RecordTransactionInitiation(
          _node_id, noc_dest, AXI_TYPE_B, _interface_dataw, _network_id);
      _i_unique_sim_id.write(unique_sim_id);
    } else {
      _i_valid.write(false);
    }
    wait();
  }
}

// Implements the packetization logic of the selected transaction into N flits,
// given that N has to be less than or equal to the fabric-to-adapter clock
// period ratio.
void aximm_master_adapter::InputPacketization() {
  // Reset Code
  _packetization_cycle.write(0);
  _num_packetization_flits.write(0);
  _injection_afifo_full.write(false);
  wait();

  while (true) {
    // If a valid transaction is selected and asynchronous injection FIFO has
    // empty slots
    if (_i_valid.read()) {
      // Packetization is simulated by generating all flits and pushing them
      // into the asynchronous injection FIFO in the first FSM packetization
      // state, then remains idle during the remaining states (analogous to an
      // assymmetric FIFO with input size of N flits & output size of 1 flit)
      if (_packetization_cycle.read() == 0) {
        sc_bv<AXI4_PAYLOADW> packet_bv;
        unsigned int num_flits = 0;
        if (_i_type.read() == AXI_TYPE_B) {

          num_flits = (unsigned int)(ceil((AXI4_RESPW + AXI4_USERW) * 1.0 /
                                          NOC_LINKS_PAYLOAD_WIDTH));
          AXI4_USER(packet_bv) = _i_user.read();
          AXI4_RESP(packet_bv) = _i_resp.read();
          Flit::FlitType flit_type = Flit::WRITE_REPLY;
          int flit_vc_id = VCIDFromType(flit_type, _noc_config);

          // Construct the head (and only) flit of B transaction
          for (unsigned int flit_id = 0; flit_id < num_flits; flit_id++) {
            sc_flit packetization_flit(flit_id == 0, flit_id == num_flits - 1,
                                       flit_type, flit_vc_id,
                                       _i_noc_dest.read(), _i_type.read(),
                                       _i_id.read(), _i_unique_sim_id.read());
            set_flit_payload(packetization_flit, packet_bv, flit_id);
            _injection_afifo.push(packetization_flit);
          }
        } else if (_i_type.read() == AXI_TYPE_R) {

          num_flits = (unsigned int)(ceil(
              (_interface_dataw + AXI4_RESPW + AXI4_USERW + 1) * 1.0 /
              NOC_LINKS_PAYLOAD_WIDTH));
          AXI4_USER(packet_bv) = _i_user.read();
          AXI4_RESP(packet_bv) = _i_resp.read();
          AXI4_LAST(packet_bv) = _i_last.read();
          AXI4_DATA(packet_bv) = _i_payload.read();
          Flit::FlitType flit_type = Flit::READ_REPLY;
          int flit_vc_id = VCIDFromType(flit_type, _noc_config);

          for (unsigned int flit_id = 0; flit_id < num_flits; flit_id++) {
            sc_flit packetization_flit(flit_id == 0, flit_id == num_flits - 1,
                                       flit_type, flit_vc_id,
                                       _i_noc_dest.read(), _i_type.read(),
                                       _i_id.read(), _i_unique_sim_id.read());
            set_flit_payload(packetization_flit, packet_bv, flit_id);
            _injection_afifo.push(packetization_flit);
          }
        }
        _num_packetization_flits.write(num_flits);
        _packetization_cycle.write(_packetization_cycle.read() + 1);
      }
    }

    if (_packetization_cycle.read() > 0) {
      uint8_t limit;
      if (_num_packetization_flits.read() >= _freq_ratio) {
        limit = _num_packetization_flits.read() - 1;
      } else {
        limit = _freq_ratio - 1;
      }
      if (_packetization_cycle.read() == limit) {
        NoCTransactionTelemetry::RecordTransactionTailPacketization(
            _i_unique_sim_id.read());
        _packetization_cycle.write(0);
      } else {
        _packetization_cycle.write(_packetization_cycle.read() + 1);
      }
    }

    // Asynchronous injection FIFO is considered full if it cannot hold the
    // maximum number of flits per transaction
    _injection_afifo_full.write(
        (_injection_afifo.size() + _max_flits_per_transaction) >
        _injection_afifo_depth);
    // std::cout << this->name() << ": " << _injection_afifo.size() <<
    // std::endl;
    wait();
  }
}

// Implements the formation of Booksim flits from SystemC flits, flit injection,
// and NoC credit processing
void aximm_master_adapter::InputInjection() {
  // Reset Code
  _injection_flit_ready = false;
  wait();

  while (true) {
    // If injection FIFO is not empty or a flit was already popped from the FIFO
    // and is ready for injection
    if (!_injection_afifo.empty() || _injection_flit_ready) {
      // If no flit was previously popped from the injection FIFO, pop one and
      // set ready flag to true
      if (!_injection_flit_ready && !_injection_afifo.empty()) {
        _to_be_injected_flit = _injection_afifo.front();
        _injection_afifo.pop();
        _injection_flit_ready = true;
      }

      // Check if Booksim NoC can accept a new flit
      if (!_buffer_state->IsFullFor(
              (int)_to_be_injected_flit._vc_id.to_uint()) &&
          (!_to_be_injected_flit._head || !_wait_for_tail_credit ||
           _buffer_state->IsAvailableFor(
               (int)_to_be_injected_flit._vc_id.to_uint()))) {

        // Construct Booksim flit
        Flit *booksim_flit = Flit::New();
        booksim_flit->id = _to_be_injected_flit._sim_transaction_id;
        booksim_flit->pid = (int)_to_be_injected_flit._packet_id.to_uint();
        booksim_flit->watch = false;
        booksim_flit->subnetwork = 0;
        booksim_flit->src = _node_id;
        booksim_flit->ctime = GetSimulationCycle(
            radsim_config->GetDoubleVectorKnob("noc_clk_period", _network_id));
        booksim_flit->itime = GetSimulationCycle(
            radsim_config->GetDoubleVectorKnob("noc_clk_period", _network_id));
        booksim_flit->cl = 0;
        booksim_flit->head = _to_be_injected_flit._head;
        booksim_flit->tail = _to_be_injected_flit._tail;
        booksim_flit->type = _to_be_injected_flit._type;
        booksim_flit->dest = _to_be_injected_flit._dest.to_uint();
        booksim_flit->dest_interface =
            _to_be_injected_flit._dest_interface.to_uint();
        booksim_flit->pri = 0;
        booksim_flit->vc = (int)_to_be_injected_flit._vc_id.to_uint();
        booksim_flit->data = _to_be_injected_flit._payload;

        // Set flit routing information
        if (_lookahead_routing) {
          const FlitChannel *inject = _noc->GetInject(booksim_flit->src);
          const Router *router = inject->GetSink();
          assert(router);
          int in_channel = inject->GetSinkPort();
          _routing_func(router, booksim_flit, in_channel,
                        &booksim_flit->la_route_set, false);
        }

        // Inject Booksim flit into the NoC
        _noc->WriteFlit(booksim_flit, booksim_flit->src);
        if (_to_be_injected_flit._head) {
          _buffer_state->TakeBuffer((int)_to_be_injected_flit._vc_id.to_uint());
        }
        _buffer_state->SendingFlit(booksim_flit);
        _injection_flit_ready = false;
        if (booksim_flit->head)
          NoCTransactionTelemetry::RecordTransactionHeadInjection(
              _to_be_injected_flit._sim_transaction_id);
        /*std::cout << this->name() << ": Injected flit to VC "
                  << (int)(_to_be_injected_flit._vc_id.to_uint())
                  << " with destination node "
                  << _to_be_injected_flit._dest.to_uint() << "!" << std::endl;*/
      }
    }

    // Check if NoC sent back a credit to this node and process it accordingly
    Credit *const c = _noc->ReadCredit(_node_id);
    if (c) {
      _buffer_state->ProcessCredit(c);
      c->Free();
    }

    wait();
  }
}