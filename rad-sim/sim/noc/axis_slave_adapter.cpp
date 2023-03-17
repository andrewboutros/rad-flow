#include <axis_slave_adapter.hpp>

axis_slave_adapter::axis_slave_adapter(
    const sc_module_name &name, int node_id, int network_id,
    std::vector<Flit::FlitType> &interface_types,
    std::vector<unsigned int> &interface_dataw, double node_period,
    double adapter_period, BookSimConfig *noc_config, Network *noc,
    BufferState *buffer_state, tRoutingFunction routing_func,
    bool lookahead_routing, bool wait_for_tail_credit)
    : sc_module(name) {
  axis_interfaces.init(interface_types.size());

  // Node properties
  _node_id = node_id;
  _network_id = network_id;
  _node_period = node_period;
  _adapter_period = adapter_period;
  _noc_period = radsim_config.GetDoubleVectorKnob("noc_period", _network_id);
  _num_axis_interfaces = interface_types.size();
  _interface_types = interface_types;
  _interface_dataw = interface_dataw;
  _num_flits_per_packet.resize(_num_axis_interfaces);
  _max_num_flits_per_packet = 0;
  _freq_ratio = (int)ceil(_node_period * 1.0 / _adapter_period);
  for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces;
       interface_id++) {
    int payload_dataw = AXIS_TRANSACTION_PAYLOAD_WIDTH - AXIS_MAX_DATAW +
                        interface_dataw[interface_id];
    _num_flits_per_packet[interface_id] =
        (int)ceil(payload_dataw * 1.0 / NOC_LINKS_PAYLOAD_WIDTH);
    _max_num_flits_per_packet =
        std::max((int)_max_num_flits_per_packet,
                 (int)_num_flits_per_packet[interface_id]);
  }

  // NoC specifications
  _noc_config = noc_config;
  _noc = noc;
  _buffer_state = buffer_state;
  _routing_func = routing_func;
  _lookahead_routing = lookahead_routing;
  _wait_for_tail_credit = wait_for_tail_credit;

  _tready_values.resize(_num_axis_interfaces, false);
  for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces;
       interface_id++)
    _axis_interface_priority.push(interface_id);

  _injection_afifo_depth =
      radsim_config.GetIntVectorKnob("adapter_fifo_size", _network_id);
  _injection_flit_ready = false;

  SC_METHOD(InputReady);
  sensitive << _injection_afifo_full << rst << _packetization_busy;
  for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces;
       interface_id++)
    sensitive << axis_interfaces[interface_id].tvalid;
  SC_CTHREAD(InputInjection, noc_clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(InputPacketization, adapter_clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(InputInterface, node_clk.pos());
  reset_signal_is(rst, true);
}

axis_slave_adapter::~axis_slave_adapter() {}

int axis_slave_adapter::GetInputDestinationNode(
    const sc_bv<AXIS_DESTW> axis_transaction_dest) {
  return (int)axis_transaction_dest.to_uint();
}

// Combinational logic for setting the tready signal of the slave NoC adapter
void axis_slave_adapter::InputReady() {
  // Set all tready values to false initially
  for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces;
       interface_id++)
    _tready_values[interface_id] = false;

  // If packetization is busy, skip arbitration logic since adapter can't accept
  // new transaction anyway
  if (!_packetization_busy.read()) {
    // Go over interfaces (in order of their current priority) -- if the
    // injection FIFO is not full, the first interface with valid data is going
    // to be accepted and its corresponding tready value is set to true
    for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces;
         interface_id++) {
      unsigned int nxt_interface_id = _axis_interface_priority.front();
      if (!_injection_afifo_full &&
          axis_interfaces[nxt_interface_id].tvalid.read()) {
        _tready_values[nxt_interface_id] = true;
        break;
      }
      _axis_interface_priority.push(nxt_interface_id);
      _axis_interface_priority.pop();
    }
  }

  while (_axis_interface_priority.front() != 0) {
    _axis_interface_priority.push(_axis_interface_priority.front());
    _axis_interface_priority.pop();
  }

  // Set tready output ports to their corresponding values
  for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces;
       interface_id++)
    axis_interfaces[interface_id].tready.write(_tready_values[interface_id]);
}

// Sequential logic for registering an AXI streaming transaction from one of the
// input interfaces -- this typically operates at the node frequency
void axis_slave_adapter::InputInterface() {
  _input_axis_transaction_id.write(0);
  _input_axis_transaction_interface.write(0);
  _input_axis_transaction.Reset();
  wait();

  while (true) {
    // Go over all interfaces checking for one with both tready and tvalid set
    // to true. If found, register this AXI streaming transaction for
    // packetization
    bool transaction_registered = false;
    for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces;
         interface_id++) {
      if (axis_interfaces[interface_id].tready.read() &&
          axis_interfaces[interface_id].tvalid.read()) {
        // Register AXI streaming transaction signals
        _input_axis_transaction.tdata.write(
            axis_interfaces[interface_id].tdata.read());
        _input_axis_transaction.tdest.write(
            axis_interfaces[interface_id].tdest.read());
        _input_axis_transaction.tid.write(
            axis_interfaces[interface_id].tid.read());
        _input_axis_transaction.tkeep.write(
            axis_interfaces[interface_id].tkeep.read());
        _input_axis_transaction.tlast.write(
            axis_interfaces[interface_id].tlast.read());
        _input_axis_transaction.tstrb.write(
            axis_interfaces[interface_id].tstrb.read());
        _input_axis_transaction.tuser.write(
            axis_interfaces[interface_id].tuser.read());
        _input_axis_transaction.tvalid.write(
            axis_interfaces[interface_id].tvalid.read());
        transaction_registered = true;

        // Log transaction initiation to get a unique packet ID
        int unique_sim_packet_id =
            NoCTransactionTelemetry::RecordTransactionInitiation(
                _node_id,
                GetInputDestinationNode(
                    axis_interfaces[interface_id].tdest.read()),
                _interface_types[interface_id], _interface_dataw[interface_id],
                _network_id);

        // Register the packet ID, transaction type, and which interface it came
        // from
        _input_axis_transaction_id.write(unique_sim_packet_id);
        _input_axis_transaction_type.write(_interface_types[interface_id]);
        _input_axis_transaction_interface.write(interface_id);

        break;
      }
    }

    // If no transactions were registered, set the valid signal of the
    // registered transaction to false
    if (!transaction_registered)
      _input_axis_transaction.tvalid.write(false);
    wait();
  }
}

// Sequential logic for packetizing a registered AXI streaming transaction
void axis_slave_adapter::InputPacketization() {
  _injection_afifo_full.write(false);
  _packetization_busy.write(false);
  _packetization_cycle = 0;
  wait();

  while (true) {
    int used_interface_id = _input_axis_transaction_interface.read();
    if (_input_axis_transaction.tvalid.read() || _packetization_busy.read()) {
      // During the first packetization cycle, form as many flits as needed by
      // this transaction and push them to the injection FIFO -- In hardware,
      // this happens over P clock cycles where a flit is formed and buffered
      // each cycle, but is modeled by doing all the work in the 1st cycle &
      // remaining idle for (P-1) cycles
      if (_packetization_cycle == 0) {
        // Put the AXI-streaming transaction in bit vector format
        sc_bv<AXIS_TRANSACTION_WIDTH> packet_bv;
        AXIS_TLAST(packet_bv) = _input_axis_transaction.tlast.read();
        AXIS_TUSER(packet_bv) = _input_axis_transaction.tuser.read();
        AXIS_TDEST(packet_bv) = _input_axis_transaction.tdest.read();
        AXIS_TID(packet_bv) = _input_axis_transaction.tid.read();
        AXIS_TDATA(packet_bv) = _input_axis_transaction.tdata.read();

        // Form flits and push them to the injection FIFO
        for (unsigned int flit_id = 0;
             flit_id < _num_flits_per_packet[used_interface_id]; flit_id++) {
          sc_flit packetization_flit(
              flit_id == 0,
              flit_id == _num_flits_per_packet[used_interface_id] - 1,
              _input_axis_transaction_type,
              VCIDFromType(_input_axis_transaction_type, _noc_config),
              AXIS_TDEST(packet_bv).to_uint(), AXIS_TID(packet_bv).to_uint(),
              _input_axis_transaction_id.read(),
              _input_axis_transaction_id.read());
          set_flit_payload(packetization_flit, packet_bv, flit_id);
          _injection_afifo.push(packetization_flit);
        }
        _packetization_cycle += 1;

        // If number of flits to be formed needs more than 1 node clock period,
        // set the packetization busy flag to be true such that the adapter does
        // not accept a new transaction before finishing the packetization of
        // the previous one
        _packetization_busy.write(_num_flits_per_packet[used_interface_id] >
                                  _freq_ratio);
      } else if (_packetization_cycle ==
                 std::max((int)_num_flits_per_packet[used_interface_id],
                          (int)_freq_ratio) -
                     1) {
        _packetization_cycle = 0;
        _packetization_busy.write(false);
        NoCTransactionTelemetry::RecordTransactionTailPacketization(
            _input_axis_transaction_id.read());
      } else {
        _packetization_cycle += 1;
      }
    }
    _injection_afifo_full.write(_injection_afifo.size() >=
                                _injection_afifo_depth -
                                    _max_num_flits_per_packet);
    wait();
  }
}

void axis_slave_adapter::InputInjection() {
  _injection_flit_ready = false;
  wait();

  while (true) {
    if (!_injection_afifo.empty() || _injection_flit_ready) {
      if (!_injection_flit_ready && !_injection_afifo.empty()) {
        _to_be_injected_flit = _injection_afifo.front();
        _injection_afifo.pop();
        _injection_flit_ready = true;
      }

      if (!_buffer_state->IsFullFor(
              (int)_to_be_injected_flit._vc_id.to_uint()) &&
          (!_to_be_injected_flit._head || !_wait_for_tail_credit ||
           _buffer_state->IsAvailableFor(
               (int)_to_be_injected_flit._vc_id.to_uint()))) {
        Flit *booksim_flit = Flit::New();
        booksim_flit->id = _to_be_injected_flit._sim_transaction_id;
        booksim_flit->watch = false;
        booksim_flit->pid = (int)(_to_be_injected_flit._packet_id.to_uint());
        booksim_flit->subnetwork = 0;
        booksim_flit->src = _node_id;
        booksim_flit->ctime = GetSimulationCycle(_noc_period);
        booksim_flit->itime = GetSimulationCycle(_noc_period);
        booksim_flit->cl = 0;
        booksim_flit->head = _to_be_injected_flit._head;
        booksim_flit->tail = _to_be_injected_flit._tail;
        booksim_flit->type = _to_be_injected_flit._type;
        booksim_flit->dest =
            GetInputDestinationNode(_to_be_injected_flit._dest);
        booksim_flit->dest_interface =
            _to_be_injected_flit._dest_interface.to_uint();
        booksim_flit->pri = 0;
        booksim_flit->vc = (int)(_to_be_injected_flit._vc_id.to_uint());
        booksim_flit->data = _to_be_injected_flit._payload;

        if (_lookahead_routing) {
          const FlitChannel *inject = _noc->GetInject(booksim_flit->src);
          const Router *router = inject->GetSink();
          assert(router);
          int in_channel = inject->GetSinkPort();
          _routing_func(router, booksim_flit, in_channel,
                        &booksim_flit->la_route_set, false);
        }

        _noc->WriteFlit(booksim_flit, booksim_flit->src);
        if (_to_be_injected_flit._head) {
          _buffer_state->TakeBuffer((int)_to_be_injected_flit._vc_id.to_uint());
        }
        _buffer_state->SendingFlit(booksim_flit);
        _injection_flit_ready = false;
        if (booksim_flit->head)
          NoCTransactionTelemetry::RecordTransactionHeadInjection(
              booksim_flit->id);
      }
    }

    Credit *const c = _noc->ReadCredit(_node_id);
    if (c) {
      _buffer_state->ProcessCredit(c);
      c->Free();
    }
    wait();
  }
}