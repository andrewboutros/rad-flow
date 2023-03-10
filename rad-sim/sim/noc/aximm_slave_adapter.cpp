#include "aximm_slave_adapter.hpp"

aximm_slave_adapter::aximm_slave_adapter(const sc_module_name& name, int node_id, int network_id,
                                         unsigned int interface_dataw, double node_period, double adapter_period,
                                         BookSimConfig *noc_config, Network *noc, BufferState *buffer_state, 
                                         tRoutingFunction routing_func, bool lookahead_routing, 
                                         bool wait_for_tail_credit, map<int, int> *ejected_flits) : sc_module(name) {
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
    _injection_afifo_depth = radsim_config.GetIntKnob("adapter_fifo_size");
    
    _freq_ratio = (int) ceil(_node_period * 1.0 / _adapter_period);
    int payload_dataw = AXI_TRANSACTION_PAYLOAD_WIDTH - AXI_MAX_DATAW + interface_dataw;
    _num_flits = (int)ceil(payload_dataw * 1.0 / NOC_LINKS_PAYLOAD_WIDTH);
    _max_flits_per_transaction = (int)ceil(AXI_TRANSACTION_PAYLOAD_WIDTH * 1.0 / NOC_LINKS_PAYLOAD_WIDTH);
    _packetization_state = 0;
    _arready = false; _awready = false; _wready = false;
    _injection_flit_ready = false; _output_packet_ready = false;
    _last_vc_id = 0;

    // Request pipeline modules described as SystemC methods and threads
    SC_METHOD(InputReady);
    sensitive << _injection_afifo_full << aximm_interface.arvalid << aximm_interface.awvalid 
        << aximm_interface.wvalid << _injection_priority_setting << rst;
    SC_CTHREAD(InputInjection, noc_clk.pos());
    reset_signal_is(rst, true);
    SC_CTHREAD(InputPacketization, adapter_clk.pos());
    reset_signal_is(rst, true);
    SC_CTHREAD(InputInterface, node_clk.pos());
    reset_signal_is(rst, true);

    // Initialize response interface (B, R) member variables
    _ejected_booksim_flit = nullptr;
    _ejection_afifo_depth = radsim_config.GetIntKnob("adapter_fifo_size");
    for(int i = 0; i < 2; i++){
        _ejection_afifos[i] = new sc_fifo<sc_flit>(_ejection_afifo_depth);
        _ejection_afifo_priority.push(i);
    }
    _output_afifo = new sc_fifo<sc_packet>(2);

    // Response pipeline modules described as SystemC methods and threads
    SC_CTHREAD(OutputEjection, noc_clk.pos());
    reset_signal_is(rst, true );
    SC_CTHREAD(OutputDepacketization, adapter_clk.pos());
    reset_signal_is(rst, true);
    SC_CTHREAD(OutputInterface, node_clk.pos());
    reset_signal_is(rst, true);
}

aximm_slave_adapter::~aximm_slave_adapter() {
    delete _noc;
    delete _noc_config;
    for(auto& ejection_fifo : _ejection_afifos){
        delete ejection_fifo;
    }
    delete _output_afifo;
}

// Implements the input ready interface based on prioritized round-robin arbitration. This decides which interface
// (AR, AW, W) is captured and pased to the packetization stage. The W interface is not selected unless an AW
// transaction was previously captured and a W transaction with "last" flag is not received yet.
void aximm_slave_adapter::InputReady() {
    if( rst.read() ){
        _arready = false;
        _awready = false;
        _wready  = false;
    } else {
        bool flag;
        // If priority setting is 0, check AR->AW->W. If priority setting is 1, check AW->W->AR. If priority setting is
        // 2, check W->AW->AR. The priority setting is changed as soon as a transaction is selected and captured.
        if (_injection_priority_setting.read() == 0) {
            _arready = !_injection_afifo_full && aximm_interface.arvalid.read();
            flag = _arready;
            _awready = !_injection_afifo_full && !flag && aximm_interface.awvalid.read();
            flag |= _awready;
            _wready = !_injection_afifo_full && !flag && aximm_interface.wvalid.read() && _got_aw.read();
        } else if (_injection_priority_setting.read() == 1) {
            _awready = !_injection_afifo_full && aximm_interface.awvalid.read();
            flag = _awready;
            _wready = !_injection_afifo_full && !flag && aximm_interface.wvalid.read() && _got_aw.read();
            flag |= _wready;
            _arready = !_injection_afifo_full && !flag && aximm_interface.arvalid.read();
        } else {
            _wready = !_injection_afifo_full && aximm_interface.wvalid.read() && _got_aw.read();
            flag = _wready;
            _arready = !_injection_afifo_full && !flag && aximm_interface.arvalid.read();
            flag |= _arready;
            _awready = !_injection_afifo_full && !flag && aximm_interface.awvalid.read();
        }

        // Verify that at most one ready signal is asserted based on the current priority setting and valid signals
        assert(((_arready ^ _awready ^ _wready) && !(_arready && _awready && _wready)) ||
               (!_arready && !_awready && !_wready));
    }
    aximm_interface.awready.write(_awready);
    aximm_interface.wready.write(_wready);
    aximm_interface.arready.write(_arready);
}

// Implements the input interface capturing into the reduced set of signals to be passed to the adapter packetization
// stage based on the set valid & ready signals.
void aximm_slave_adapter::InputInterface() {
    // Reset
    _i_id.write(0);
    _i_payload.write(0);
    _i_ctrl.write(0);
    _i_user.write(0);
    _i_valid.write(false);
    _i_type.write(0);
    _i_unique_sim_id.write(0);
    _injection_priority_setting.write(0);
    _got_aw.write(false);
    wait();

    while ( true ) {
        sc_bv<AXI_CTRL> tmp_ctrl;
        if (aximm_interface.arready.read() && aximm_interface.arvalid.read()) {
            // Capture the values at the AXI AR port
            _i_id.write(aximm_interface.arid.read());
            _i_payload.write(aximm_interface.araddr.read());
            AXI_CTRL_LEN(tmp_ctrl) = aximm_interface.arlen.read();
            AXI_CTRL_SIZE(tmp_ctrl) = aximm_interface.arsize.read();
            AXI_CTRL_BURST(tmp_ctrl) = aximm_interface.arburst.read();
            _i_ctrl.write(tmp_ctrl);
            _i_user.write(aximm_interface.aruser.read());
            _i_valid.write(aximm_interface.arvalid.read());
            _i_type.write(AR_TRANSACTION);

            // Adjust priority setting and log the initiation of an AR transaction
            _injection_priority_setting.write(1);

            // Record initiation time in simulation statistics
            sc_bv<AXI_ADDRW> araddr = aximm_interface.araddr.read();
            int unique_sim_packet_id = NoCTransactionTelemetry::RecordTransactionInitiation(
              _node_id, (int) InputAddressTranslation(araddr).to_uint(), AR_TRANSACTION, 
              _interface_dataw, _network_id);
            _i_unique_sim_id.write(unique_sim_packet_id);
        } else if (aximm_interface.awready.read() && aximm_interface.awvalid.read()) {
            // Capture the values at the AXI AW port
            _i_id.write(aximm_interface.awid.read());
            _i_payload = aximm_interface.awaddr.read();
            AXI_CTRL_LEN(tmp_ctrl) = aximm_interface.awlen.read();
            AXI_CTRL_SIZE(tmp_ctrl) = aximm_interface.awsize.read();
            AXI_CTRL_BURST(tmp_ctrl) = aximm_interface.awburst.read();
            _i_ctrl.write(tmp_ctrl);
            _i_user.write(aximm_interface.awuser.read());
            _i_valid.write(aximm_interface.awvalid.read());
            _i_type.write(AW_TRANSACTION);
            _got_aw.write(true);

            // Adjust priority setting and log the initiation of an AW transaction
            _injection_priority_setting.write(2);

            // Record initiation time in simulation statistics
            sc_bv<AXI_ADDRW> araddr = aximm_interface.araddr.read();
            int unique_sim_packet_id = NoCTransactionTelemetry::RecordTransactionInitiation(
              _node_id, (int) InputAddressTranslation(araddr).to_uint(), AW_TRANSACTION,
              _interface_dataw, _network_id);
            _i_unique_sim_id.write(unique_sim_packet_id);
        } else if (aximm_interface.wready.read() && aximm_interface.wvalid.read() && _got_aw.read()) {
            // Capture the values at the AXI W port
            _i_id.write(aximm_interface.wid.read());
            _i_payload.write(aximm_interface.wdata.read());
            tmp_ctrl.range(AXI_CTRL - 1, 1) = 0;
            tmp_ctrl.range(0, 0) = aximm_interface.wlast.read();
            _i_ctrl.write(tmp_ctrl);
            _i_user.write(aximm_interface.wuser.read());
            _i_valid.write(aximm_interface.wvalid.read());
            _i_type.write(W_TRANSACTION);
            _got_aw.write(!aximm_interface.wlast.read());

            // Adjust priority setting and log the initiation of an W transaction
            _injection_priority_setting.write(0);

            // Record initiation time in simulation statistics
            int unique_sim_packet_id = NoCTransactionTelemetry::RecordTransactionInitiation(
              _node_id, (int) _last_awaddr.read().to_uint(), W_TRANSACTION,
              _interface_dataw, _network_id);
            _i_unique_sim_id.write(unique_sim_packet_id);
        } else {
            _i_valid.write(false);
        }
        wait();
    }
}

// Implements the translation of input address to destination node. Any address translation scheme is allowable.
// The current implementation splits the 64-bit address space into num_nodes equal portions (i.e. destination node is
// specified by the N most-significant address bits, such that num_nodes = log2(N))
sc_bv<AXI_DESTW> aximm_slave_adapter::InputAddressTranslation(sc_bv<AXI_ADDRW>& addr){
    return addr.range(AXI_ADDRW - 1, AXI_ADDRW - AXI_DESTW);
}

void set_flit_payload(sc_flit &packetization_flit, sc_bv<AXI_TRANSACTION_PAYLOAD_WIDTH> &packet_bv, int flit_id) {
  unsigned int start_idx = flit_id * NOC_LINKS_PAYLOAD_WIDTH;
  unsigned int end_idx = std::min((flit_id + 1) * NOC_LINKS_PAYLOAD_WIDTH, AXI_TRANSACTION_PAYLOAD_WIDTH);
  *(packetization_flit._payload) = packet_bv.range(end_idx - 1, start_idx);
}

// Implements the packetization logic of the selected transaction into N flits, given that N has to be less than or
// equal to the fabric-to-adapter clock period ratio.
void aximm_slave_adapter::InputPacketization() {
    // Reset Code
    _injection_afifo_full.write(false);
    _last_awaddr.write(0);
    wait();

    while ( true ) {
        // If a valid transaction is selected and asynchronous injection FIFO has empty slots
        if (_i_valid.read() && (_injection_afifo->num_free() > 0)) {
            // Packetization is simulated by generating all flits and pushing them into the asynchronous injection
            // FIFO in the first FSM packetization state, then remains idle during the remaining states (analogous
            // to an assymmetric FIFO with input size of N flits & output size of 1 flit)
            if (_packetization_cycle == 0) {
                int num_flits = 0;
                if( (_i_type.read() == AR_TRANSACTION) || (_i_type.read() == AW_TRANSACTION) ) {
                    // Check if minimum node-to-adapter frequency ratio is satisfied
                    num_flits = (int) (ceil(AXI_ADDRW * 1.0 / NOC_LINKS_PAYLOAD_WIDTH));
                    if (num_flits > _max_flits_per_transaction) {
                        cerr << "Error: Minimum node-to-adapter frequency ratio not satisfied!" << endl;
                        exit(1);
                    }

                    // Construct the head flit of AW/AR transaction
                    sc_bv<AXI_TRANSACTION_PAYLOAD_WIDTH> packet_bv;
                    AXI_TRANSACTION_CTRL(packet_bv) = _i_ctrl.read();
                    AXI_TRANSACTION_DATA(packet_bv) = _i_payload.read();
                    AXI_TRANSACTION_ID(packet_bv) = _i_id.read();
                    AXI_TRANSACTION_USER(packet_bv) = _i_user.read();
                    
                    unsigned int vc_id = _i_type.read(); // FIXME
                    unsigned int dest = 0; // FIXME
                    Flit::FlitType type = 0; // FIXME
                    if(_i_type.read() == AW_TRANSACTION){
                      _last_awaddr.write(dest);
                    }

                    for (unsigned int flit_id = 0; flit_id < num_flits; flit_id++) {
                      sc_flit packetization_flit(flit_id == 0, flit_id == num_flits - 1, type,
                        VCIDFromType(type, _noc_config), dest, 0, _i_id.read(), _i_unique_sim_id.read());
                      set_flit_payload(packetization_flit, packet_bv, flit_id);
                      _injection_afifo.push(packetization_flit);
                    }
                } else if ( _i_type.read() == W_TRANSACTION ) {
                    // Check if minimum node-to-adapter frequency ratio is satisfied
                    num_flits = (int) (ceil(_interface_dataw * 1.0 / DATAW));
                    if( num_flits > _max_flits_per_transaction ){
                        cerr << "Error: Minimum node-to-adapter frequency ratio not satisfied!" << endl;
                        exit(1);
                    }

                    // Construct the head flit of AW/AR transaction
                    sc_flit head_flit(true, (num_flits == 1), _i_type.read(),
                                      _i_id.read(), _i_unique_sim_id.read());
                    CTRL(head_flit.payload) = _i_ctrl.read().range(0, 0);
                    DEST(head_flit.payload) = _last_awaddr.read();
                    SRC(head_flit.payload) = this->_node_id;
                    DATA(head_flit.payload) = _i_payload.read().range(min(DATAW, _dataw_config) - 1, 0);
                    _injection_afifo->nb_write(head_flit);
                    LogFlitInjection(head_flit, _node_id, SimConfig::GetDoubleKnob("fabric_period"));

                    // Construct remaining flits, if exist
                    for (int i = 1; i < num_flits; i++) {
                        sc_flit body_flit(false, (i == num_flits - 1), _i_type.read(),
                                          _i_id.read(), _i_unique_sim_id.read());
                        DATA(body_flit.payload) = _i_payload.read().range(min(_dataw_config, ((i+1)*DATAW))-1,
                                                                          i*DATAW);
                        _injection_afifo->nb_write(body_flit);
                        LogFlitInjection(body_flit, _node_id, SimConfig::GetDoubleKnob("fabric_period"));
                    }
                }
                SimStats::RecordTransactionTailPacketization( _i_unique_sim_id.read(), num_flits );
            }
            // Advance packetization state, and wrap around back to 0 for processing next packet
            _packetization_state = (_packetization_state == _max_flits_per_transaction - 1) ?
                    0 : _packetization_state + 1;
        }
        // Asynchronous injection FIFO is considered full if it cannot hold the maximum number of flits per transaction
        _injection_afifo_full.write( _injection_afifo->num_free() <= _max_flits_per_transaction );
        wait();
    }
}

// Implements the mapping of a given flit to an NoC virtual channel. Any custom VC mapping scheme can be implemented
// in this function and used by the adapter injection logic
int aximm_slave_adapter::VCMapping( sc_flit& flit ){
    if( flit.head ){
        _last_vc_id = (int) (flit.type.to_uint());
    }
    return _last_vc_id;
}

// Implements the formation of Booksim flits from SystemC flits, flit injection, and NoC credit processing.
void aximm_slave_adapter::InputInjection() {
    //Reset Code
    _injection_flit_ready = false;
    wait();

    while ( true ) {
        // If injection FIFO is not empty or a flit was already poped from the FIFO and is ready for injection
        if (_injection_afifo->num_available() > 0 || _injection_flit_ready) {
            // If no flit was previously poped from the injection FIFO, pop one and set ready flag to true
            if( !_injection_flit_ready && _injection_afifo->num_available() > 0) {
                _injection_afifo->nb_read(_to_be_injected_flit);
                _to_be_injected_flit.vc_id = VCMapping( _to_be_injected_flit );
                _injection_flit_ready = true;
            }

            // Check if Booksim NoC can accept a new flit
            if( !_buffer_state->IsFullFor((int)_to_be_injected_flit.vc_id.to_uint()) &&
                (!_to_be_injected_flit.head || !_wait_for_tail_credit ||
                _buffer_state->IsAvailableFor((int)_to_be_injected_flit.vc_id.to_uint()))) {

                // Construct Booksim flit
                Flit *booksim_flit = Flit::New();
                booksim_flit->id = _to_be_injected_flit.sim_transaction_id;
                booksim_flit->pid = (int) (_to_be_injected_flit.packet_id.to_uint());
                booksim_flit->watch = false;
                booksim_flit->subnetwork = 0;
                booksim_flit->src = _node_id;
                booksim_flit->ctime = GetSimulationCycle(SimConfig::GetDoubleKnob("noc_period"));
                booksim_flit->itime = GetSimulationCycle(SimConfig::GetDoubleKnob("noc_period"));
                booksim_flit->cl = 0;
                booksim_flit->head = _to_be_injected_flit.head;
                booksim_flit->tail = _to_be_injected_flit.tail;
                switch(_to_be_injected_flit.type.to_uint()){
                    case AR_TRANSACTION: booksim_flit->type = Flit::READ_REQUEST;  break;
                    case AW_TRANSACTION: booksim_flit->type = Flit::WRITE_REQUEST; break;
                    case W_TRANSACTION:  booksim_flit->type = Flit::WRITE_DATA;    break;
                }
                booksim_flit->dest = DEST(_to_be_injected_flit.payload).to_uint();
                booksim_flit->pri = 0;
                booksim_flit->vc = (int) (_to_be_injected_flit.vc_id.to_uint());
                booksim_flit->data = _to_be_injected_flit.payload;

                // Set flit routing information
                if (_lookahead_routing) {
                    const FlitChannel *inject = _noc->GetInject(booksim_flit->src);
                    const Router *router = inject->GetSink();
                    assert(router);
                    int in_channel = inject->GetSinkPort();
                    _routing_func(router, booksim_flit, in_channel, &booksim_flit->la_route_set, false);
                }

                // Inject Booksim Flit into the NoC
                _noc->WriteFlit(booksim_flit, booksim_flit->src);
                if(_to_be_injected_flit.head) {
                    _buffer_state->TakeBuffer((int) _to_be_injected_flit.vc_id.to_uint());
                }
                _buffer_state->SendingFlit(booksim_flit);
                _injection_flit_ready = false;
                if(booksim_flit->head)
                    SimStats::RecordTransactionHeadInjection( booksim_flit->id );

                LogBooksimFlitInjection(booksim_flit, _node_id, SimConfig::GetDoubleKnob("fabric_period"));
            }
        }

        // Check if NoC sent back a credit to this node and process it accordingly
        Credit * const c = _noc->ReadCredit( _node_id );
        if ( c ) {
            _buffer_state->ProcessCredit(c);
            c->Free();
        }
        wait();
    }
}

// Implements the inverse mapping of virtual channel to the type of flit received on this channel. Any custom mapping
// scheme can be implemented in this function and used in the flit ejection logic.
int aximm_slave_adapter::InverseVCMapping(int vc_id) {
    // Get the start and end VCs of R and B transactions from Booksim NoC configuration
    int r_vc_start = _noc_config->GetInt("read_reply_begin_vc");
    int r_vc_end = _noc_config->GetInt("read_reply_end_vc");
    int b_vc_start = _noc_config->GetInt("write_reply_begin_vc");
    int b_vc_end = _noc_config->GetInt("write_reply_end_vc");

    // Categorize the received flit based on its VC ID: 0 for R transaction, 1 for B transaction
    if(vc_id >= r_vc_start && vc_id <= r_vc_end){
        return 0;
    } else if(vc_id >= b_vc_start && vc_id <= b_vc_end){
        return 1;
    } else {
        cerr << "Response VC ID (" << vc_id << ") does not correspond to a response VC" << endl;
        exit(1);
    }
}

// Implements the ejection of Booksim flits from the NoC and the injection into VC buffers according to the flit type.
// Each VC buffer accepts flits until a complete packet can be formed.
void aximm_slave_adapter::OutputEjection() {
    // Reset Code
    for(auto & ejection_fifo_push_counter : _ejection_afifo_push_counter) {
        ejection_fifo_push_counter.write(0);
    }
    wait();

    while( true ){
        // If previously ejected flit was pused to a fifo, try read a new one
        if( _ejected_booksim_flit == nullptr ){
            _ejected_booksim_flit = _noc->ReadFlit(_node_id);
        }

        // If reading a flit from the NoC is successful, start processing it
        if( _ejected_booksim_flit ) {
            // Check if corresponding ejection FIFO can accept a new flit
            int vc_id = _ejected_booksim_flit->vc;
            int type_id = InverseVCMapping( vc_id );
            if( _ejection_afifos[type_id]->num_free() > 0 ) {
                // Create a SystemC flit and push it to its corresponding ejection FIFO
                sc_flit ejected_flit(_ejected_booksim_flit->head, _ejected_booksim_flit->tail,
                                     _ejected_booksim_flit->type, _ejected_booksim_flit->pid,
                                     _ejected_booksim_flit->id);
                ejected_flit.vc_id = vc_id;
                ejected_flit.payload = static_cast<sc_bv<PAYLD>*>(_ejected_booksim_flit->data);
                _ejection_afifos[type_id]->nb_write(ejected_flit);

                // If tail flit received, increment FIFO counter (i.e. FIFO has 1 more complete packet), and record
                // ejection time
                if ( _ejected_booksim_flit->tail ) {
                    int updated_count = _ejection_afifo_push_counter[type_id].read() + 1;
                    _ejection_afifo_push_counter[type_id].write(updated_count);
                    SimStats::RecordTransactionTailEjection( _ejected_booksim_flit->id );
                    SimStats::UpdateHops( _ejected_booksim_flit->id, _ejected_booksim_flit->hops );
                }

                // Log flit receipt and set _ejected_booksim_flit to NULL to indicate it was processed
                LogBooksimFlitEjection( _ejected_booksim_flit, _node_id,
                                        SimConfig::GetDoubleKnob("fabric_period"));
                _ejected_booksim_flit = nullptr;
            }
        }
        wait();
    }
}

// Implements depacketization logic of flits from the VC buffer holding a complete packet. The re-constructed packet is
// then pushed to the output double buffer (i.e. FIFO of size 2).
void aximm_slave_adapter::OutputDepacketization() {
    // Reset Code
    for(auto & ejection_afifo_pop_counter : _ejection_afifo_pop_counter){
        ejection_afifo_pop_counter.write( 0 );
    }
    _ejection_afifo_is_depacketizing.write( false );

    while( true ){
        unsigned int highest_priority_fifo_id = _ejection_afifo_priority.front();
        bool fifo_has_complete_packet = _ejection_afifo_push_counter[highest_priority_fifo_id].read() >
                                        _ejection_afifo_pop_counter[highest_priority_fifo_id].read();

        // If no ejection FIFO is currently being depacketized
        if( !_ejection_afifo_is_depacketizing.read() ) {

            // Loop over ejection FIFOs based on current priority status
            // By the end of this while loop: If fifo_has_complete_packet is true, the highest priority FIFO index
            // will be at the top of the priority queue. If fifo_has_complete_packet is false, no FIFO has a complete
            // packet ready
            unsigned int num_checked_fifos = 0;
            while (!fifo_has_complete_packet && num_checked_fifos < NUM_RSP_TYPES) {
                _ejection_afifo_priority.push(_ejection_afifo_priority.front());
                _ejection_afifo_priority.pop();
                num_checked_fifos++;
                highest_priority_fifo_id = _ejection_afifo_priority.front();
                fifo_has_complete_packet = _ejection_afifo_push_counter[highest_priority_fifo_id].read() >
                                           _ejection_afifo_pop_counter[highest_priority_fifo_id].read();
            }
        }

        // If there is a complete packet, set depacketization flag and start processing it
        if ( fifo_has_complete_packet ) {
            // If output double buffer (FIFO of size 2) has free space, depacketize flits
            if (_output_afifo->num_free() > 0) {
                // Pop a flit from the highest priority ejection FIFO
                auto* processed_flit = new sc_flit();
                _ejection_afifos[highest_priority_fifo_id]->nb_read( *processed_flit );
                _constructed_packet.AddFlit( processed_flit );

                // Write credit for the accepted flit to the corresponding VC
                _ejected_flits->insert(make_pair(_node_id, processed_flit->vc_id.to_uint()));

                // If the flit is a tail, push constructed packet to output double buffer & reset depacketization flag
                if( processed_flit->tail ){
                    _output_afifo->nb_write( _constructed_packet );
                    SimStats::RecordTransactionTailDepacketization( processed_flit->sim_transaction_id );
                    _constructed_packet.Reset();
                    _ejection_afifo_is_depacketizing.write( false );

                    // Update FIFO packet pop counter
                    int updated_count = _ejection_afifo_pop_counter[highest_priority_fifo_id].read() + 1;
                    _ejection_afifo_pop_counter[highest_priority_fifo_id].write(updated_count);
                    assert( updated_count <= _ejection_afifo_push_counter[highest_priority_fifo_id] );

                    // Adjust ejection FIFOs priority status
                    _ejection_afifo_priority.push(_ejection_afifo_priority.front());
                    _ejection_afifo_priority.pop();
                } else {
                    _ejection_afifo_is_depacketizing.write( true );
                }
            } else {
                _ejection_afifo_is_depacketizing.write( true );
            }
        }
        wait();
    }
}

// Implements the steering of the re-constructed packet to the corresponding output interface based on its type.
void aximm_slave_adapter::OutputInterface() {
    // Reset
    axi_mm.rvalid.write( false );
    axi_mm.bvalid.write( false );
    _output_packet_ready = false;
    wait();

    while ( true ) {
        if( _output_afifo->num_available() > 0 && !_output_packet_ready ){
            _output_afifo->nb_read( _output_packet );
            _output_packet_ready = true;
        }
        if( _output_packet_ready && (_output_packet.GetFlit(0)->type == B_TRANSACTION) && axi_mm.bready.read() ) {
            sc_bv<IDW + AXI_BWIDTH> temp_bv;
            sc_flit *temp_flit = _output_packet.GetFlit(0);
            temp_bv.range(IDW + AXI_BWIDTH - 1, AXI_BWIDTH) = temp_flit->packet_id;
            for (int i = 0; i < _output_packet.GetNumValidFlits(); i++) {
                temp_bv.range(min(AXI_BWIDTH, (i+1)*DATAW)-1, i*DATAW) =
                        _output_packet.GetFlit(i)->payload->range(min(AXI_BWIDTH, (i+1)*DATAW)-1, i*DATAW);
            }
            axi_mm.rvalid.write( false );
            axi_mm.bvalid.write(true);
            axi_mm.bid.write(temp_bv.range(IDW + AXI_BWIDTH - 1, AXI_BWIDTH).to_uint());
            axi_mm.bresp.write(temp_bv.range(AXI_BWIDTH - 1, 0).to_uint());
            LogTransactionReceipt( temp_bv, _node_id, SimConfig::GetDoubleKnob("fabric_period") );
            _output_packet_ready = false;
            SimStats::RecordTransactionReceipt( temp_flit->sim_transaction_id );
        } else if( _output_packet_ready && (_output_packet.GetFlit(0)->type == R_TRANSACTION) &&
                axi_mm.rready.read() ){
            sc_bv<IDW + AXI_RWIDTH + MAX_DATAW> temp_bv;
            sc_flit *temp_flit = _output_packet.GetFlit(0);
            temp_bv.range(IDW + AXI_RWIDTH + MAX_DATAW - 1, AXI_RWIDTH + MAX_DATAW) = temp_flit->packet_id;
            temp_bv.range(AXI_RWIDTH + MAX_DATAW - 1, MAX_DATAW) = CTRL(temp_flit->payload).range(2,0);
            for (int i = 0; i < _output_packet.GetNumValidFlits(); i++) {
                temp_bv.range(min(MAX_DATAW, (i + 1) * DATAW) - 1, i * DATAW) =
                        DATA(_output_packet.GetFlit(i)->payload);
            }
            axi_mm.bvalid.write( false );
            axi_mm.rvalid.write(true);
            axi_mm.rid.write(temp_bv.range(IDW+AXI_RWIDTH+MAX_DATAW-1, AXI_RWIDTH+MAX_DATAW).to_uint());
            axi_mm.rresp.write(temp_bv.range(AXI_RWIDTH+MAX_DATAW-1, MAX_DATAW+1).to_int());
            axi_mm.rlast.write(temp_bv.range(MAX_DATAW, MAX_DATAW).to_int());
            axi_mm.rdata.write(temp_bv.range(MAX_DATAW-1, 0));
            LogTransactionReceipt( temp_bv, _node_id, SimConfig::GetDoubleKnob("fabric_period") );
            _output_packet_ready = false;
            SimStats::RecordTransactionReceipt( temp_flit->sim_transaction_id );
        } else {
            axi_mm.rvalid.write( false );
            axi_mm.bvalid.write( false );
        }
        wait();
    }
}