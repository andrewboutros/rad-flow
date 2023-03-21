#include "aximm_slave_adapter.hpp"

std::unordered_map<unsigned int, std::pair<int, std::pair<int, int>>> stats;

aximm_slave_adapter::aximm_slave_adapter(
    const sc_module_name &name, int node_id, int network_id,
    BookSimConfig *noc_config, Network *noc, BufferState *buffer_state,
    tRoutingFunction routing_func, bool lookahead_routing,
    bool wait_for_tail_credit, map<int, int> *ejected_flits,
    unsigned int interface_dataw, double node_period, double adapter_period)
    : sc_module(name) {

  // Initialize basic adapter member variables
  _node_id = node_id;
  _network_id = network_id;
  _node_period = node_period;
  _adapter_period = adapter_period;
  _noc_period = radsim_config.GetDoubleVectorKnob("noc_period", _network_id);
  _interface_dataw = interface_dataw;

  _noc_config = noc_config;
  _noc = noc;
  _buffer_state = buffer_state;
  _routing_func = routing_func;
  _lookahead_routing = lookahead_routing;
  _wait_for_tail_credit = wait_for_tail_credit;
  _ejected_flits = ejected_flits;

  // Initialize request interface (AR, AW, W) member variables
  _injection_afifo_depth =
      radsim_config.GetIntVectorKnob("adapter_fifo_size", _network_id);

  _axi_transaction_width = AXI4_USERW;
  if ((AXI4_ADDRW + AXI_CTRLW) > (_interface_dataw + AXI4_RESPW + 1)) {
    _axi_transaction_width += (AXI4_ADDRW + AXI_CTRLW);
  } else {
    _axi_transaction_width += (_interface_dataw + AXI4_RESPW + 1);
  }
  _max_flits_per_transaction =
      (int)ceil(_axi_transaction_width / NOC_LINKS_PAYLOAD_WIDTH);
  _freq_ratio = (int)floor(_node_period / _adapter_period);
  _arready = false;
  _awready = false;
  _wready = false;
  _injection_flit_ready = false;
  _output_packet_ready = false;
  _last_vc_id = 0;

  // Request pipeline modules described as SystemC methods and threads
  SC_METHOD(InputReady);
  sensitive << _injection_afifo_full << aximm_interface.arvalid
            << aximm_interface.awvalid << aximm_interface.wvalid
            << _injection_priority_setting << _packetization_cycle << rst;
  SC_CTHREAD(InputInjection, noc_clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(InputPacketization, adapter_clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(InputInterface, node_clk.pos());
  reset_signal_is(rst, true);

  // Initialize response interface (B, R) member variables
  _ejected_booksim_flit = nullptr;
  _ejection_afifo_depth =
      radsim_config.GetIntVectorKnob("adapter_fifo_size", _network_id);
  _ejection_afifos.resize(AXI_NUM_RSP_TYPES);
  _ejection_afifo_push_counter.init(AXI_NUM_RSP_TYPES);
  _ejection_afifo_pop_counter.init(AXI_NUM_RSP_TYPES);
  for (int i = 0; i < AXI_NUM_RSP_TYPES; i++) {
    _ejection_afifo_priority.push(i);
  }

  // Response pipeline modules described as SystemC methods and threads
  SC_CTHREAD(OutputEjection, noc_clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(OutputDepacketization, adapter_clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(OutputInterface, node_clk.pos());
  reset_signal_is(rst, true);
}

aximm_slave_adapter::~aximm_slave_adapter() {
  delete _noc;
  delete _noc_config;
}

// Implements the input ready interface based on prioritized round-robin
// arbitration. This decides which interface (AR, AW, W) is captured and pased
// to the packetization stage. The W interface is not selected unless an AW
// transaction was previously captured and a W transaction with "last" flag is
// not received yet.
void aximm_slave_adapter::InputReady() {
  if (rst.read()) {
    _arready = false;
    _awready = false;
    _wready = false;
  } else {
    bool flag;
    // If priority setting is 0, check AR->AW->W. If priority setting is 1,
    // check AW->W->AR. If priority setting is 2, check W->AW->AR. The priority
    // setting is changed as soon as a transaction is selected and captured.
    if (_injection_priority_setting.read() == 0) {
      _arready = !_injection_afifo_full && aximm_interface.arvalid.read();
      flag = _arready;
      _awready =
          !_injection_afifo_full && !flag && aximm_interface.awvalid.read();
      flag |= _awready;
      _wready = !_injection_afifo_full && !flag &&
                aximm_interface.wvalid.read() && _got_aw.read();
    } else if (_injection_priority_setting.read() == 1) {
      _awready = !_injection_afifo_full && aximm_interface.awvalid.read();
      flag = _awready;
      _wready = !_injection_afifo_full && !flag &&
                aximm_interface.wvalid.read() && _got_aw.read();
      flag |= _wready;
      _arready =
          !_injection_afifo_full && !flag && aximm_interface.arvalid.read();
    } else {
      _wready = !_injection_afifo_full && aximm_interface.wvalid.read() &&
                _got_aw.read();
      flag = _wready;
      _arready =
          !_injection_afifo_full && !flag && aximm_interface.arvalid.read();
      flag |= _arready;
      _awready =
          !_injection_afifo_full && !flag && aximm_interface.awvalid.read();
    }

    // Verify that at most one ready signal is asserted based on the current
    // priority setting and valid signals
    assert(((_arready ^ _awready ^ _wready) &&
            !(_arready && _awready && _wready)) ||
           (!_arready && !_awready && !_wready));
  }
  aximm_interface.awready.write((_packetization_cycle.read() == 0) && _awready);
  aximm_interface.wready.write((_packetization_cycle.read() == 0) && _wready);
  aximm_interface.arready.write((_packetization_cycle.read() == 0) && _arready);
}

// Implements the input interface capturing into the reduced set of signals to
// be passed to the adapter packetization stage based on the set valid & ready
// signals.
void aximm_slave_adapter::InputInterface() {
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
  _got_aw.write(false);
  wait();

  while (true) {
    sc_bv<AXI_CTRLW> tmp_ctrl;
    if (aximm_interface.arready.read() && aximm_interface.arvalid.read()) {
      // Capture the values at the AXI AR port
      _i_id.write(aximm_interface.arid.read());
      _i_payload.write(aximm_interface.araddr.read());
      unsigned int offset = 0;
      tmp_ctrl.range(offset + AXI4_BURSTW - 1, offset) =
          aximm_interface.arburst.read();
      offset += AXI4_BURSTW;
      tmp_ctrl.range(offset + AXI4_SIZEW - 1, offset) =
          aximm_interface.arsize.read();
      offset += AXI4_SIZEW;
      tmp_ctrl.range(offset + AXI4_LENW - 1, offset) =
          aximm_interface.arlen.read();
      _i_ctrl.write(tmp_ctrl);
      _i_user.write(aximm_interface.aruser.read());
      _i_valid.write(aximm_interface.arvalid.read());
      _i_type.write(AXI_TYPE_AR);
      sc_bv<AXI4_ADDRW> araddr = aximm_interface.araddr.read();
      int noc_dest = GetInputDestinationNode(araddr);
      _i_noc_dest.write(noc_dest);

      // Adjust priority setting and log the initiation of an AR transaction
      _injection_priority_setting.write(1);

      // Record initiation time in simulation statistics
      int unique_sim_id = NoCTransactionTelemetry::RecordTransactionInitiation(
          _node_id, noc_dest, AXI_TYPE_AR, _interface_dataw, _network_id);
      _i_unique_sim_id.write(unique_sim_id);
      // std::cout << this->name() << ": Registered AR transaction (user = "
      //           << aximm_interface.aruser.read().to_uint64() << ")!"
      //           << std::endl;
    } else if (aximm_interface.awready.read() &&
               aximm_interface.awvalid.read()) {
      // Capture the values at the AXI AW port
      _i_id.write(aximm_interface.awid.read());
      _i_payload = aximm_interface.awaddr.read();
      unsigned int offset = 0;
      tmp_ctrl.range(offset + AXI4_BURSTW - 1, offset) =
          aximm_interface.awburst.read();
      offset += AXI4_BURSTW;
      tmp_ctrl.range(offset + AXI4_SIZEW - 1, offset) =
          aximm_interface.awsize.read();
      offset += AXI4_SIZEW;
      tmp_ctrl.range(offset + AXI4_LENW - 1, offset) =
          aximm_interface.awlen.read();
      _i_ctrl.write(tmp_ctrl);
      _i_user.write(aximm_interface.awuser.read());
      _i_valid.write(aximm_interface.awvalid.read());
      _i_type.write(AXI_TYPE_AW);
      sc_bv<AXI4_ADDRW> awaddr = aximm_interface.awaddr.read();
      int noc_dest = GetInputDestinationNode(awaddr);
      _i_noc_dest.write(noc_dest);
      _got_aw.write(true);

      // Adjust priority setting and log the initiation of an AW transaction
      _injection_priority_setting.write(2);

      // Record initiation time in simulation statistics
      int unique_sim_id = NoCTransactionTelemetry::RecordTransactionInitiation(
          _node_id, noc_dest, AXI_TYPE_AW, _interface_dataw, _network_id);
      _i_unique_sim_id.write(unique_sim_id);
      /*std::cout << this->name() << ": Registered AW transaction (user = "
                << aximm_interface.awuser.read().to_uint64() << ")!"
                << std::endl;*/
    } else if (aximm_interface.wready.read() && aximm_interface.wvalid.read() &&
               _got_aw.read()) {
      // Capture the values at the AXI W port
      _i_id.write(aximm_interface.wid.read());
      _i_payload.write(aximm_interface.wdata.read());
      tmp_ctrl.range(AXI_CTRLW - 1, 1) = 0;
      tmp_ctrl.range(0, 0) = aximm_interface.wlast.read();
      _i_ctrl.write(tmp_ctrl);
      _i_user.write(aximm_interface.wuser.read());
      _i_valid.write(aximm_interface.wvalid.read());
      _i_type.write(AXI_TYPE_W);
      _i_last.write(aximm_interface.wlast.read());
      _i_noc_dest.write(_last_awdest.read());
      _got_aw.write(!aximm_interface.wlast.read());

      // Adjust priority setting and log the initiation of an W transaction
      _injection_priority_setting.write(0);

      // Record initiation time in simulation statistics
      int unique_sim_id = NoCTransactionTelemetry::RecordTransactionInitiation(
          _node_id, (int)_last_awdest.read().to_uint(), AXI_TYPE_W,
          _interface_dataw, _network_id);
      _i_unique_sim_id.write(unique_sim_id);
      /*std::cout << this->name() << ": Registered W transaction (user = "
                << aximm_interface.wuser.read().to_uint64() << ")!"
                << std::endl;*/
    } else {
      _i_valid.write(false);
    }
    wait();
  }
}

// Implements the translation of input address to destination node. Any address
// translation scheme is allowable. The current implementation splits the 64-bit
// address space into num_nodes equal portions (i.e. destination node is
// specified by the N most-significant address bits, such that num_nodes =
// log2(N))
int aximm_slave_adapter::GetInputDestinationNode(sc_bv<AXI4_ADDRW> &addr) {
  return addr.range(AXI4_ADDRW - 1, AXI4_ADDRW - NOC_LINKS_DEST_WIDTH)
      .to_uint();
}

// Implements the packetization logic of the selected transaction into N flits,
// given that N has to be less than or equal to the fabric-to-adapter clock
// period ratio.
void aximm_slave_adapter::InputPacketization() {
  // Reset Code
  _packetization_cycle.write(0);
  _num_packetization_flits.write(0);
  _injection_afifo_full.write(false);
  _last_awdest.write(0);
  wait();

  while (true) {
    // If a valid transaction is selected and asynchronous injection FIFO has
    // empty slots
    if (_i_valid.read() && (_injection_afifo.size() < _injection_afifo_depth)) {
      // Packetization is simulated by generating all flits and pushing them
      // into the asynchronous injection FIFO in the first FSM packetization
      // state, then remains idle during the remaining states (analogous to an
      // assymmetric FIFO with input size of N flits & output size of 1 flit)
      if (_packetization_cycle.read() == 0) {
        // std::cout << this->name() << ": Packetized transaction!" <<
        // std::endl;
        sc_bv<AXI_TRANSACTION_MAX_WIDTH> packet_bv;
        unsigned int num_flits = 0;
        if ((_i_type.read() == AXI_TYPE_AR) ||
            (_i_type.read() == AXI_TYPE_AW)) {

          num_flits =
              (unsigned int)(ceil((AXI4_ADDRW + AXI_CTRLW + AXI4_USERW) * 1.0 /
                                  NOC_LINKS_PAYLOAD_WIDTH));
          AXI_USER(packet_bv) = _i_user.read();
          AXI_CTRL(packet_bv) = _i_ctrl.read();
          AXI_ADDR(packet_bv) = _i_payload.read();
          Flit::FlitType flit_type = (_i_type.read() == AXI_TYPE_AR)
                                         ? Flit::READ_REQUEST
                                         : Flit::WRITE_REQUEST;
          int flit_vc_id = VCIDFromType(flit_type, _noc_config);

          for (unsigned int flit_id = 0; flit_id < num_flits; flit_id++) {
            sc_flit packetization_flit(flit_id == 0, flit_id == num_flits - 1,
                                       flit_type, flit_vc_id,
                                       _i_noc_dest.read(), _i_type.read(),
                                       _i_id.read(), _i_unique_sim_id.read());
            set_flit_payload(packetization_flit, packet_bv, flit_id);
            _injection_afifo.push(packetization_flit);
          }

          if (_i_type.read() == AXI_TYPE_AW) {
            sc_bv<AXI4_ADDRW> addr = _i_payload.read();
            _last_awdest.write(GetInputDestinationNode(addr));
          }
        } else if (_i_type.read() == AXI_TYPE_W) {

          num_flits = (unsigned int)(ceil(
              (_interface_dataw + AXI4_RESPW + AXI4_USERW + 1) * 1.0 /
              NOC_LINKS_PAYLOAD_WIDTH));
          AXI_USER(packet_bv) = _i_user.read();
          AXI_RESP(packet_bv) = 0;
          AXI_LAST(packet_bv) = _i_last.read();
          AXI_DATA(packet_bv) = _i_payload.read();
          Flit::FlitType flit_type = Flit::WRITE_DATA;
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
    wait();
  }
}

// Implements the formation of Booksim flits from SystemC flits, flit injection,
// and NoC credit processing.
void aximm_slave_adapter::InputInjection() {
  // Reset Code
  _injection_flit_ready = false;
  wait();

  while (true) {
    // If injection FIFO is not empty or a flit was already poped from the FIFO
    // and is ready for injection
    if (!_injection_afifo.empty() > 0 || _injection_flit_ready) {
      // If no flit was previously poped from the injection FIFO, pop one and
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
        booksim_flit->pid = (int)(_to_be_injected_flit._packet_id.to_uint());
        booksim_flit->watch = false;
        booksim_flit->subnetwork = 0;
        booksim_flit->src = _node_id;
        booksim_flit->ctime = GetSimulationCycle(
            radsim_config.GetDoubleVectorKnob("noc_period", _network_id));
        booksim_flit->itime = GetSimulationCycle(
            radsim_config.GetDoubleVectorKnob("noc_period", _network_id));
        booksim_flit->cl = 0;
        booksim_flit->head = _to_be_injected_flit._head;
        booksim_flit->tail = _to_be_injected_flit._tail;
        booksim_flit->type = _to_be_injected_flit._type;
        booksim_flit->dest = _to_be_injected_flit._dest.to_uint();
        booksim_flit->dest_interface =
            _to_be_injected_flit._dest_interface.to_uint();
        booksim_flit->pri = 0;
        booksim_flit->vc = (int)(_to_be_injected_flit._vc_id.to_uint());
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

        // Inject Booksim Flit into the NoC
        _noc->WriteFlit(booksim_flit, booksim_flit->src);
        if (_to_be_injected_flit._head) {
          _buffer_state->TakeBuffer((int)_to_be_injected_flit._vc_id.to_uint());
        }
        _buffer_state->SendingFlit(booksim_flit);
        _injection_flit_ready = false;
        if (booksim_flit->head)
          NoCTransactionTelemetry::RecordTransactionHeadInjection(
              booksim_flit->id);
        /*std::cout << this->name() << ": Injected flit to VC "
                  << (int)(_to_be_injected_flit._vc_id.to_uint()) << "!"
                  << std::endl;*/
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

// Implements the inverse mapping of virtual channel to the type of flit
// received on this channel. Any custom mapping scheme can be implemented in
// this function and used in the flit ejection logic.
int aximm_slave_adapter::InverseVCMapping(int vc_id) {
  // Get the start and end VCs of R and B transactions from Booksim NoC
  // configuration
  int r_vc_start = _noc_config->GetInt("read_reply_begin_vc");
  int r_vc_end = _noc_config->GetInt("read_reply_end_vc");
  int b_vc_start = _noc_config->GetInt("write_reply_begin_vc");
  int b_vc_end = _noc_config->GetInt("write_reply_end_vc");

  // Categorize the received flit based on its VC ID: 0 for R transaction, 1 for
  // B transaction
  if (vc_id >= r_vc_start && vc_id <= r_vc_end) {
    return AXI_TYPE_R;
  } else if (vc_id >= b_vc_start && vc_id <= b_vc_end) {
    return AXI_TYPE_B;
  } else {
    cerr << "Response VC ID (" << vc_id
         << ") does not correspond to a response VC" << endl;
    exit(1);
  }
}

// Implements the ejection of Booksim flits from the NoC and the injection into
// VC buffers according to the flit type. Each VC buffer accepts flits until a
// complete packet can be formed.
void aximm_slave_adapter::OutputEjection() {
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
      int type_id = (InverseVCMapping(vc_id) == AXI_TYPE_R) ? 0 : 1;
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
        // complete packet), and record ejection time
        if (_ejected_booksim_flit->tail) {
          int updated_count = _ejection_afifo_push_counter[type_id].read() + 1;
          _ejection_afifo_push_counter[type_id].write(updated_count);
          NoCTransactionTelemetry::RecordTransactionTailEjection(
              _ejected_booksim_flit->id);
          NoCTransactionTelemetry::UpdateHops(_ejected_booksim_flit->id,
                                              _ejected_booksim_flit->hops);
        }

        // Set _ejected_booksim_flit to NULL to indicate it was processed
        _ejected_booksim_flit = nullptr;
      }
    }
    wait();
  }
}

// Implements depacketization logic of flits from the VC buffer holding a
// complete packet. The re-constructed packet is then pushed to the output
// double buffer (i.e. FIFO of size 2).
void aximm_slave_adapter::OutputDepacketization() {
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
             num_checked_fifos < AXI_NUM_RSP_TYPES) {
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

// Implements the steering of the re-constructed packet to the corresponding
// output interface based on its type.
void aximm_slave_adapter::OutputInterface() {
  // Reset
  aximm_interface.rvalid.write(false);
  aximm_interface.bvalid.write(false);
  _output_packet_ready = false;
  wait();

  while (true) {
    if (!_output_afifo.empty() && !_output_packet_ready) {
      _output_packet = _output_afifo.front();
      _output_afifo.pop();
      _output_packet_ready = true;
    }

    if (_output_packet_ready &&
        (_output_packet.GetFlit(0)->_type == Flit::FlitType::WRITE_REPLY) &&
        aximm_interface.bready.read()) {
      sc_bv<AXI_TRANSACTION_MAX_WIDTH> temp_bv;
      sc_flit *temp_flit = _output_packet.GetFlit(0);

      for (unsigned int i = 0; i < _output_packet.GetNumValidFlits(); i++) {
        unsigned int start_idx = i * NOC_LINKS_PAYLOAD_WIDTH;
        unsigned int end_idx = min((int)AXI_TRANSACTION_MAX_WIDTH,
                                   (int)(i + 1) * NOC_LINKS_PAYLOAD_WIDTH);
        sc_bv<NOC_LINKS_PAYLOAD_WIDTH> flit_payload =
            *(_output_packet.GetFlit(i)->_payload);
        temp_bv.range(end_idx - 1, start_idx) = flit_payload;
      }

      aximm_interface.bid.write(temp_flit->_packet_id);
      aximm_interface.rvalid.write(false);
      aximm_interface.bvalid.write(true);
      aximm_interface.buser.write(AXI_USER(temp_bv));
      aximm_interface.bresp.write(AXI_RESP(temp_bv));
      _output_packet_ready = false;
      NoCTransactionTelemetry::RecordTransactionReceipt(
          temp_flit->_sim_transaction_id);
    } else if (_output_packet_ready &&
               (_output_packet.GetFlit(0)->_type ==
                Flit::FlitType::READ_REPLY) &&
               aximm_interface.rready.read()) {

      sc_bv<AXI_TRANSACTION_MAX_WIDTH> temp_bv;
      sc_flit *temp_flit = _output_packet.GetFlit(0);

      for (unsigned int i = 0; i < _output_packet.GetNumValidFlits(); i++) {
        unsigned int start_idx = i * NOC_LINKS_PAYLOAD_WIDTH;
        unsigned int end_idx = min((int)AXI_TRANSACTION_MAX_WIDTH,
                                   (int)(i + 1) * NOC_LINKS_PAYLOAD_WIDTH);
        sc_bv<NOC_LINKS_PAYLOAD_WIDTH> flit_payload =
            *(_output_packet.GetFlit(i)->_payload);
        temp_bv.range(end_idx - 1, start_idx) = flit_payload;
      }

      aximm_interface.bvalid.write(false);
      aximm_interface.rvalid.write(true);
      aximm_interface.rid.write(temp_flit->_packet_id);
      aximm_interface.rresp.write(AXI_RESP(temp_bv).to_uint());
      aximm_interface.rlast.write(AXI_LAST(temp_bv).to_uint());
      aximm_interface.rdata.write(AXI_DATA(temp_bv));
      aximm_interface.ruser.write(AXI_USER(temp_bv));
      _output_packet_ready = false;
      NoCTransactionTelemetry::RecordTransactionReceipt(
          temp_flit->_sim_transaction_id);

      // std::cout << GetSimulationCycle(5.0) << " === " << this->name()
      //           << ": Got R response!" << std::endl;
    } else {
      aximm_interface.rvalid.write(false);
      aximm_interface.bvalid.write(false);
    }
    wait();
  }
}