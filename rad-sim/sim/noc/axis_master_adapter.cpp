#include "axis_master_adapter.hpp"

axis_master_adapter::axis_master_adapter(
    const sc_module_name &name, int node_id, int network_id,
    std::vector<Flit::FlitType> &interface_types,
    std::vector<unsigned int> &interface_dataw, BookSimConfig *noc_config,
    Network *noc, BufferState *buffer_state, tRoutingFunction routing_func,
    bool lookahead_routing, bool wait_for_tail_credit,
    map<int, int> *ejected_flits)
    : sc_module(name) {
  _node_id = node_id;
  _network_id = network_id;
  _num_axis_interfaces = interface_types.size();
  _num_flits.resize(_num_axis_interfaces);
  _max_num_flits =
      (int)ceil(AXIS_TRANSACTION_PAYLOAD_WIDTH * 1.0 / NOC_LINKS_PAYLOAD_WIDTH);
  _interface_dataw = interface_dataw;
  for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces;
       interface_id++) {
    int payload_dataw = AXIS_TRANSACTION_PAYLOAD_WIDTH - AXIS_MAX_DATAW +
                        interface_dataw[interface_id];
    _num_flits[interface_id] =
        (int)ceil(payload_dataw * 1.0 / NOC_LINKS_PAYLOAD_WIDTH);
    ;
  }
  _num_vcs = radsim_config.GetIntVectorKnob("noc_vcs", _network_id);
  axis_interfaces.init(_num_axis_interfaces);

  _noc_config = noc_config;
  _noc = noc;
  _buffer_state = buffer_state;
  _routing_func = routing_func;
  _lookahead_routing = lookahead_routing;
  _wait_for_tail_credit = wait_for_tail_credit;
  _ejected_flits = ejected_flits;

  _ejected_booksim_flit = nullptr;
  _ejection_afifo_depth =
      radsim_config.GetIntVectorKnob("adapter_fifo_size", _network_id);
  _ejection_afifos.resize(_num_vcs);
  _ejection_afifo_push_counter.init(_num_vcs);
  _ejection_afifo_pop_counter.init(_num_vcs);
  for (unsigned int vc_id = 0; vc_id < _num_vcs; vc_id++) {
    _ejection_afifo_priority.push(vc_id);
  }

  _output_afifos.resize(_num_axis_interfaces);
  _output_packet_ready.resize(_num_axis_interfaces);
  _output_afifo_depth =
      radsim_config.GetIntVectorKnob("adapter_obuff_size", _network_id);
  _constructed_packet = sc_packet();
  _output_chunk.resize(_num_axis_interfaces);

  SC_CTHREAD(OutputEjection, noc_clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(OutputDepacketization, adapter_clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(OutputInterface, node_clk.pos());
  reset_signal_is(rst, true);
}

axis_master_adapter::~axis_master_adapter() {
  delete _noc;
  delete _noc_config;
}

void axis_master_adapter::OutputEjection() {
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
      if (_ejection_afifos[vc_id].size() < _ejection_afifo_depth) {
        // Create a SystemC flit and push it to its corresponding ejection FIFO
        sc_flit ejected_flit(
            _ejected_booksim_flit->head, _ejected_booksim_flit->tail,
            _ejected_booksim_flit->type, _ejected_booksim_flit->vc,
            _ejected_booksim_flit->dest, _ejected_booksim_flit->dest_interface,
            _ejected_booksim_flit->pid, _ejected_booksim_flit->id);
        ejected_flit._payload = static_cast<sc_bv<NOC_LINKS_PAYLOAD_WIDTH> *>(
            _ejected_booksim_flit->data);
        ejected_flit._vc_id = vc_id;
        _ejection_afifos[vc_id].push(ejected_flit);

        // If tail flit received, increment FIFO counter (i.e. FIFO has 1 more
        // complete packet)
        if (_ejected_booksim_flit->tail) {
          NoCTransactionTelemetry::RecordTransactionTailEjection(
              _ejected_booksim_flit->id);
          NoCTransactionTelemetry::UpdateHops(_ejected_booksim_flit->id,
                                              _ejected_booksim_flit->hops);
          int updated_count = _ejection_afifo_push_counter[vc_id].read() + 1;
          _ejection_afifo_push_counter[vc_id].write(updated_count);
        }

        // Set _ejected_booksim_flit to NULL to indicate it was processed
        _ejected_booksim_flit = nullptr;
      }
    }
    wait();
  }
}

void axis_master_adapter::OutputDepacketization() {
  // Reset Code
  for (auto &ejection_afifo_pop_counter : _ejection_afifo_pop_counter) {
    ejection_afifo_pop_counter.write(0);
  }
  _ejection_afifo_is_depacketizing.write(false);

  while (true) {
    unsigned int highest_priority_fifo_id = _ejection_afifo_priority.front();
    unsigned int dest_output_fifo = 0;
    if (!_ejection_afifos[highest_priority_fifo_id].empty()) {
      dest_output_fifo = _ejection_afifos[highest_priority_fifo_id]
                             .front()
                             ._dest_interface.to_uint();
    }
    bool fifo_has_complete_packet =
        (_ejection_afifo_push_counter[highest_priority_fifo_id].read() >
         _ejection_afifo_pop_counter[highest_priority_fifo_id].read()) &&
        _output_afifos[dest_output_fifo].size() < _output_afifo_depth;

    // If no ejection FIFO is currently being depacketized
    if (!_ejection_afifo_is_depacketizing.read()) {
      // Loop over ejection FIFOs based on current priority status
      // By the end of this while loop: If fifo_has_complete_packet is true, the
      // highest priority FIFO index will be at the top of the priority queue.
      // If fifo_has_complete_packet is false, no FIFO has a complete packet
      // ready
      unsigned int num_checked_fifos = 0;
      while (!fifo_has_complete_packet && num_checked_fifos < _num_vcs) {
        _ejection_afifo_priority.push(_ejection_afifo_priority.front());
        _ejection_afifo_priority.pop();
        num_checked_fifos++;
        highest_priority_fifo_id = _ejection_afifo_priority.front();
        if (!_ejection_afifos[highest_priority_fifo_id].empty()) {
          dest_output_fifo = _ejection_afifos[highest_priority_fifo_id]
                                 .front()
                                 ._dest_interface.to_uint();
          fifo_has_complete_packet =
              (_ejection_afifo_push_counter[highest_priority_fifo_id].read() >
               _ejection_afifo_pop_counter[highest_priority_fifo_id].read()) &&
              _output_afifos[dest_output_fifo].size() < _output_afifo_depth;
        }
      }
    }

    // If there is a complete packet, set depacketization flag and start
    // processing it if (fifo_has_complete_packet) { unsigned int
    // dest_output_fifo =
    // _ejection_afifos[highest_priority_fifo_id].front()._dest_interface.to_uint();
    // If output double buffer (FIFO of size 2) has free space, depacketize
    // flits
    if (_output_afifos[dest_output_fifo].size() < _output_afifo_depth &&
        fifo_has_complete_packet) {
      // Pop a flit from the highest priority ejection FIFO
      sc_flit processed_flit =
          _ejection_afifos[highest_priority_fifo_id].front();
      _ejection_afifos[highest_priority_fifo_id].pop();
      _constructed_packet.AddFlit(processed_flit);
      if (processed_flit._head) {
        sc_bv<NOC_LINKS_PAYLOAD_WIDTH> head_payload =
            *(processed_flit._payload);
        _constructed_packet.SetStreamID(AXIS_TID(head_payload).to_uint());
      }
      _ejected_flits->insert(make_pair(_node_id, highest_priority_fifo_id));

      // If the flit is a tail, push constructed packet to output double buffer
      // & reset depacketization flag
      if (processed_flit._tail) {
        _output_afifos[dest_output_fifo].push(_constructed_packet);
        _constructed_packet.Reset();
        NoCTransactionTelemetry::RecordTransactionTailDepacketization(
            processed_flit._sim_transaction_id);
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
      processed_flit.FreeFlit();
    } else {
      _ejection_afifo_priority.push(_ejection_afifo_priority.front());
      _ejection_afifo_priority.pop();
    }

    wait();
  }
}

void axis_master_adapter::write_sc_packet_to_axis_output(
    sc_packet &packet, axis_master_port &axis_port, int output_chunk,
    unsigned int interface_id) {
  sc_bv<AXIS_TRANSACTION_WIDTH> packet_bv;
  sc_bv<AXIS_TRANSACTION_PAYLOAD_WIDTH> packet_payload_bv;
  int start_idx, end_idx;
  for (unsigned int flit_id = 0; flit_id < packet.GetNumValidFlits();
       flit_id++) {
    if (flit_id == 0) {
      AXIS_TDEST(packet_bv) = packet.GetFlit(flit_id)->_dest;
    }
    start_idx = flit_id * NOC_LINKS_PAYLOAD_WIDTH;
    end_idx = std::min(AXIS_TRANSACTION_PAYLOAD_WIDTH,
                       (int)((flit_id + 1) * NOC_LINKS_PAYLOAD_WIDTH));
    packet_payload_bv.range(end_idx - 1, start_idx) =
        packet.GetFlit(flit_id)->_payload->range(NOC_LINKS_PAYLOAD_WIDTH - 1,
                                                 0);
  }
  AXIS_TID(packet_bv) = AXIS_TID(packet_payload_bv);
  AXIS_TLAST(packet_bv) = AXIS_TLAST(packet_payload_bv);
  AXIS_TUSER(packet_bv) = AXIS_TUSER(packet_payload_bv);
  int data_start_idx = output_chunk * _interface_dataw[interface_id];
  int data_end_idx = data_start_idx + _interface_dataw[interface_id];
  data_end_idx = std::min(data_end_idx, AXIS_MAX_DATAW);
  AXIS_TDATA(packet_bv).range(_interface_dataw[interface_id] - 1, 0) =
      AXIS_TDATA(packet_payload_bv).range(data_end_idx - 1, data_start_idx);
  AXIS_TDATA(packet_bv).range(AXIS_MAX_DATAW - 1,
                              _interface_dataw[interface_id]) = 0;

  // Write to AXI-streaming output interface
  axis_port.tvalid.write(true);
  axis_port.tdata.write(AXIS_TDATA(packet_bv));
  if (AXIS_TLAST(packet_bv) == 1)
    axis_port.tlast.write(true);
  else
    axis_port.tlast.write(false);
  axis_port.tid.write(AXIS_TID(packet_bv));
  axis_port.tdest.write(AXIS_TDEST(packet_bv));
  axis_port.tuser.write(AXIS_TUSER(packet_bv));
  axis_port.tstrb.write(0);
  axis_port.tkeep.write(0);
}

void axis_master_adapter::OutputInterface() {
  // Reset
  for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces;
       interface_id++) {
    axis_interfaces[interface_id].Reset();
    _output_packet_ready[interface_id] = false;
    _output_chunk[interface_id] = 0;
  }
  wait();

  while (true) {
    for (unsigned int interface_id = 0; interface_id < _num_axis_interfaces;
         interface_id++) {
      if (axis_interfaces[interface_id].tready.read() &&
          axis_interfaces[interface_id].tvalid.read()) {
        sc_packet output_packet = _output_afifos[interface_id].front();
        NoCTransactionTelemetry::RecordTransactionReceipt(
            output_packet.GetFlit(0)->_sim_transaction_id);
        _output_chunk[interface_id]++;

        int total_num_flits = output_packet.GetNumValidFlits();
        int num_chunks =
            (int)ceil(total_num_flits * 1.0 / _num_flits[interface_id]);
        if (_output_chunk[interface_id] == num_chunks) {
          for (unsigned int flit_id = 0;
               flit_id < output_packet.GetNumValidFlits(); flit_id++) {
            output_packet.GetFlit(flit_id)->FreeFlit();
          }
          _output_afifos[interface_id].pop();
          _output_chunk[interface_id] = 0;
        }
      }

      if (!_output_afifos[interface_id].empty()) {
        sc_packet output_packet = _output_afifos[interface_id].front();
        write_sc_packet_to_axis_output(
            output_packet, axis_interfaces[interface_id],
            _output_chunk[interface_id], interface_id);
      } else {
        axis_interfaces[interface_id].tvalid.write(false);
      }
    }

    wait();
  }
}