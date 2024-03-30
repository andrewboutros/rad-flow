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
  _noc_period = radsim_config.GetDoubleVectorKnob("noc_clk_period", _network_id);
  _num_axis_interfaces = interface_types.size();
  _interface_types = interface_types;
  _interface_dataw = interface_dataw;
  _num_flits_per_packet.resize(_num_axis_interfaces);
  _max_num_flits_per_packet = 0;
  _freq_ratio = (int)ceil(_node_period * 1.0 / _adapter_period);
  for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces;
       interface_id++) {
    int payload_dataw =
        AXIS_PAYLOADW - AXIS_MAX_DATAW + interface_dataw[interface_id];
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

  _input_axis_transactions_afifo_depth = 2;

  _injection_afifo_depth =
      radsim_config.GetIntVectorKnob("noc_adapters_fifo_size", _network_id);
  _injection_flit_ready = false;

  SC_METHOD(InputReady);
  sensitive << _injection_afifo_full << rst << _packetization_cycle << _input_axis_transactions_afifo_full;
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
  for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces; interface_id++)
    _tready_values[interface_id] = false;

  // Go over interfaces (in order of their current priority) -- if the
  // injection FIFO is not full, the first interface with valid data is going
  // to be accepted and its corresponding tready value is set to true
  for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces; interface_id++) {
    unsigned int nxt_interface_id = _axis_interface_priority.front();
    if (!_injection_afifo_full && axis_interfaces[nxt_interface_id].tvalid.read()) {
      _tready_values[nxt_interface_id] = true;
      break;
    }
    _axis_interface_priority.push(nxt_interface_id);
    _axis_interface_priority.pop();
  }

  while (_axis_interface_priority.front() != 0) {
    _axis_interface_priority.push(_axis_interface_priority.front());
    _axis_interface_priority.pop();
  }

  // Set tready output ports to their corresponding values
  //std::cout << GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period")) << ": " << this->name() << std::endl;
  for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces; interface_id++)
    axis_interfaces[interface_id].tready.write(!_input_axis_transactions_afifo_full.read() && _tready_values[interface_id]);
}

// Sequential logic for registering an AXI streaming transaction from one of the
// input interfaces -- this typically operates at the node frequency
void axis_slave_adapter::InputInterface() {
  _input_axis_transactions_afifo_full.write(false);
  wait();

  while (true) {
    // Go over all interfaces checking for one with both tready and tvalid set
    for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces; interface_id++) {
      if (axis_interfaces[interface_id].tready.read() && axis_interfaces[interface_id].tvalid.read()) {
        // Create an AXIS transaction struct
        axis_transaction input_transaction;
        input_transaction.tdata = axis_interfaces[interface_id].tdata.read();
        input_transaction.tdest = axis_interfaces[interface_id].tdest.read();
        input_transaction.tid = axis_interfaces[interface_id].tid.read();
        input_transaction.tkeep = axis_interfaces[interface_id].tkeep.read();
        input_transaction.tlast = axis_interfaces[interface_id].tlast.read();
        input_transaction.tstrb = axis_interfaces[interface_id].tstrb.read();
        input_transaction.tuser = axis_interfaces[interface_id].tuser.read();
        
        // Log transaction initiation to get a unique packet ID
        int unique_sim_packet_id =
            NoCTransactionTelemetry::RecordTransactionInitiation(
                _node_id,
                GetInputDestinationNode(axis_interfaces[interface_id].tdest.read()),
                _interface_types[interface_id], _interface_dataw[interface_id],
                _network_id);

        // Register the packet ID, transaction type, and which interface it came from
        input_transaction.transaction_id = unique_sim_packet_id;
        input_transaction.transaction_type = _interface_types[interface_id];
        input_transaction.transaction_interface = interface_id;

        _input_axis_transactions_afifo.push(input_transaction);
        break;
      }
    }
    _input_axis_transactions_afifo_full.write(_input_axis_transactions_afifo.size() >=
      _input_axis_transactions_afifo_depth);
    assert(_input_axis_transactions_afifo.size() <= _input_axis_transactions_afifo_depth);
    wait();
  }
}

// Sequential logic for packetizing a registered AXI streaming transaction
void axis_slave_adapter::InputPacketization() {
  _injection_afifo_full.write(false);
  _packetization_cycle.write(0);
  wait();

  while (true) {
    // If the input AXIS transation FIFO has pending transaction, process them
    if (!_input_axis_transactions_afifo.empty()) {
      axis_transaction input_transaction = _input_axis_transactions_afifo.front();
      int used_interface_id = input_transaction.transaction_interface;
      unsigned int num_flits = _num_flits_per_packet[used_interface_id];
      bool injection_fifo_full = !(_injection_afifo.size() < _injection_afifo_depth);

      // Form a bitvector with the AXIS transaction payload
      sc_bv<AXIS_PAYLOADW> packet_bv;
      AXIS_TLAST(packet_bv) = input_transaction.tlast;
      AXIS_TUSER(packet_bv) = input_transaction.tuser;
      AXIS_TDATA(packet_bv) = input_transaction.tdata;

      // If injection FIFO is not full, create a flit and push it to the FIFO
      if (!injection_fifo_full) {
        sc_flit packetization_flit(
            _packetization_cycle.read() == 0,
            _packetization_cycle.read() == num_flits - 1,
            input_transaction.transaction_type,
            VCIDFromType(input_transaction.transaction_type, _noc_config),
            input_transaction.tdest,
            input_transaction.tid,
            input_transaction.transaction_id,
            input_transaction.transaction_id);
        set_flit_payload(packetization_flit, packet_bv, _packetization_cycle.read());
        _injection_afifo.push(packetization_flit);

        // If all flits of a packet are created, pop the transaction from queue
        if (_packetization_cycle.read() == num_flits - 1) {
          _packetization_cycle.write(0);
          NoCTransactionTelemetry::RecordTransactionTailPacketization(
              input_transaction.transaction_id);
          _input_axis_transactions_afifo.pop();
        } else {
          _packetization_cycle.write(_packetization_cycle.read() + 1);
        }
      }
    }

    _injection_afifo_full.write(_injection_afifo.size() >= _injection_afifo_depth);
    wait();
  }
}

// Sequential logic for injecting flits into the Booksim NoC
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