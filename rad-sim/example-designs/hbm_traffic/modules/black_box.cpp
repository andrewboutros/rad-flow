#include <black_box.hpp>



black_box::black_box(const sc_module_name &name, 
        hw_module &module_conf,
        unsigned int line_dataw,
        unsigned int element_bitwidth,
        unsigned int fifos_depth) : RADSimModule(name) {

    _dataw = line_dataw;
    _bitwidth = element_bitwidth;
    _fifos_depth = fifos_depth;


    // init sc vectors
    // _mem_req_ififos_full.init(NUM_MEM_REQ_FIFOS);
    // _mem_req_ififos_empty.init(NUM_MEM_REQ_FIFOS);
    _mem_req_rd_ififo.depth = fifos_depth;
    _mem_req_wr_ififo.depth = fifos_depth;

    // use the same dataw for input and output bitwidths (can be changed)
    _num_input_elements = line_dataw / element_bitwidth;
    _num_output_elements = line_dataw / element_bitwidth;
    
    // Combinational logic and its sensitivity list
    SC_METHOD(Assign);
    sensitive << rst; 
    sensitive << _rd_data_ififo_full;
    // instruction fifos
    sensitive << _mem_req_rd_ififo.full;
    sensitive << _mem_req_wr_ififo.full;

    // Sequential logic and its clock/reset setup
    SC_CTHREAD(Tick, clk.pos());
    reset_signal_is(rst, true); // Reset is active high

    // This function must be defined & called for any RAD-Sim module to register
    // its info for automatically connecting to the NoC
    this->RegisterModuleInfo(module_conf);
    _debug_black_box_out = new ofstream("black_box.out");
}


black_box::~black_box() {
    delete _debug_black_box_out;
}


void black_box::mem_req_rx(
    sc_in<bool> &mem_req_valid,
    sc_out<bool> &mem_req_ready) {

    // if the current mem_req input is valid and the axi is ready to accept a new transaction
    if (mem_req_valid.read() && mem_req_ready.read()) {

        mem_req_inst cur_mem_req;
        cur_mem_req.target_address = target_address.read();
        cur_mem_req.target_channel = target_channel.read();
        cur_mem_req.wr_en = wr_en.read();
        cur_mem_req.wr_data = wr_data.read();
        cur_mem_req.src_port = src_port.read();
        cur_mem_req.dst_port = dst_port.read();

        /*
        mem_req_inst* cur_mem_req = new mem_req_inst;
        // load the input fifo vals to our struct containing mem req inst fields
        
        cur_mem_req->target_address.write(target_address.read());
        cur_mem_req->target_channel.write(target_channel.read());
        cur_mem_req->wr_en.write(wr_en.read());
        cur_mem_req->wr_data.write(wr_data.read());
        cur_mem_req->src_port.write(src_port.read());
        cur_mem_req->dst_port.write(dst_port.read());
        */

        if (cur_mem_req.wr_en) {
            std::cout << module_name << ": Received memory write request" << " @ Cycle " <<  GetSimulationCycle(5.0) << "!" << std::endl;
            _mem_req_wr_ififo.fifo.push(cur_mem_req);
        }
        else {
            std::cout << module_name << ": Received memory read request" << " @ Cycle " <<  GetSimulationCycle(5.0) << "!" << std::endl;
            _mem_req_rd_ififo.fifo.push(cur_mem_req);
        }
    }
}

void black_box::Assign() {
    if (rst) {
        // Traffic gen
        mem_req_ready.write(true);
        // mem_rd_req_ready.write(true);
        // mem_wr_req_ready.write(true);

        // SISO
        aximm_interface.bready.write(false);
        aximm_interface.rready.write(false);
    } else {
        // Set ready signals to accept read/write response from the AXI-MM NoC
        bool all_rd_fifos_not_full = true;
        bool all_wr_fifos_not_full = true;

        all_rd_fifos_not_full &= !_mem_req_rd_ififo.full.read();
        all_wr_fifos_not_full &= !_mem_req_wr_ififo.full.read();

        // if either of them are full then we are not ready, TODO make this smarter
        mem_req_ready.write(all_rd_fifos_not_full && all_wr_fifos_not_full);

        // mem_rd_req_ready.write(all_rd_fifos_not_full);
        // mem_wr_req_ready.write(all_wr_fifos_not_full);

        // SISO
        aximm_interface.bready.write(true); // used to be false but I think I want to it true, write resp channel
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
    _mem_req_rd_ififo.full.write(false);
    _mem_req_rd_ififo.empty.write(true);
    _mem_req_wr_ififo.full.write(false);
    _mem_req_wr_ififo.empty.write(true);

    // for (int i = 0; i < _mem_rd_req_ififos_full.size(); i++) {
    //     _mem_rd_req_ififos_full[i].write(false);
    //     _mem_rd_req_ififos_empty[i].write(true);
    // }
    // for (int i = 0; i < _mem_wr_req_ififos_full.size(); i++) {
    //     _mem_wr_req_ififos_full[i].write(false);
    //     _mem_wr_req_ififos_empty[i].write(true);
    // }


    _wr_req_id_count.write(0);
    _wr_data_id_count.write(0);
    _rd_req_id_count.write(0);

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

    // State of AXI-MM transactions
    _aximm_wr_num_sent_flits.write(0);
    _aximm_wr_tx_done.write(true);
    _aximm_rd_tx_done.write(true);
    _aximm_wr_state.write(AXIMM_WR_IDLE);



    // Verif
    rd_req_data.write(0);
    rd_req_data_rdy.write(false);
    wr_req_data.write(0);
    rd_req_data_rdy.write(false);

    wait();

    // Always @ positive edge of the clock
    while (true) {

        // Traffic Gen
        // if the current mem_req input is valid and the axi is ready to accept a new transaction
        
        // Take in RD / WR requests and put them into appropriate fifo
        if (!_mem_req_rd_ififo.full.read() && !_mem_req_wr_ififo.full.read()) {
            mem_req_rx(mem_req_valid, mem_req_ready);
        }

        //   ___ ___   _   ___  
        //  | _ \ __| /_\ |   \ 
        //  |   / _| / _ \| |) |
        //  |_|_\___/_/ \_\___/ 
        switch (_aximm_rd_state.read()){
            case AXIMM_RD_IDLE:
                if (!_mem_req_rd_ififo.empty) {
                    _aximm_rd_state.write(AXIMM_RD_ADDR);
                    std::cout << "Sent RD Trans id " << _rd_req_id_count.read() << " ADDR + CTRL" << " @ Cycle " <<  GetSimulationCycle(5.0) << "!" << std::endl;
                    
                    mem_req_inst cur_mem_req = _mem_req_rd_ififo.fifo.front();
                    uint64_t dst_addr = cur_mem_req.dst_port + cur_mem_req.target_address;

                    // AXI-MM Read
                    // Address + Control Channel Ready
                    aximm_interface.arvalid.write(true);
                    aximm_interface.arid.write(_rd_req_id_count.read());
                    aximm_interface.araddr.write(dst_addr);
                    aximm_interface.arlen.write(0); // burst len of 1
                    aximm_interface.arsize.write(0); // burst len of 1
                    aximm_interface.arburst.write(0); // burst len of 1
                    aximm_interface.aruser.write(cur_mem_req.src_port); // assuming the aw user field specifies port where the request came from?
                    
                    // aximm_interface.wvalid.write(false);
                    // aximm_interface.awvalid.write(false);
                } else {
                    _aximm_rd_state.write(AXIMM_RD_IDLE);
                    // aximm_interface.arvalid.write(false); // deassert addr valid if not sending a transaction , stay in AXIMM_RD_IDLE
                }
                break;
            case AXIMM_RD_ADDR:
                // If the Address + Control Transaction was recieved
                if (aximm_interface.arvalid.read() && aximm_interface.arready.read()){
                    assert (!_mem_req_rd_ififo.empty); // make sure input fifo is not empty
                    aximm_interface.arvalid.write(false); // deassert address read valid
                    // Move back to idle state and deassert address read valid
                    _aximm_rd_state.write(AXIMM_RD_IDLE);
                    std::cout << "Sent AR transaction @ id: " << _rd_req_id_count.read() << std::endl;
                    _mem_req_rd_ififo.fifo.pop();
                    _rd_req_id_count.write(_rd_req_id_count.read() + 1);
                    // _aximm_rd_tx_done.write(true);
                }   
                break;
            default:
                break;
        }

        //  __      _____ ___ _____ ___ 
        //  \ \    / / _ \_ _|_   _| __|
        //   \ \/\/ /|   /| |  | | | _| 
        //    \_/\_/ |_|_\___| |_| |___|
        mem_req_inst cur_wr_mem_req;
        uint64_t dst_addr;
        switch (_aximm_wr_state.read()){
            case AXIMM_WR_IDLE:
            {
                // reset the write Verif signals
                wr_req_data_rdy.write(false);

                if (!_mem_req_wr_ififo.empty.read() && !aximm_interface.bvalid.read()){
                    // Address + Control 
                    _aximm_wr_state.write(AXIMM_WR_ADDR);
                    std::cout << "Sent WR Trans id " << _wr_req_id_count.read() << " ADDR + CTRL " << " @ Cycle " <<  GetSimulationCycle(5.0) << "!" << std::endl;

                    cur_wr_mem_req = _mem_req_wr_ififo.fifo.front();
                    dst_addr = cur_wr_mem_req.dst_port + cur_wr_mem_req.target_address;

                    aximm_interface.awvalid.write(true);
                    aximm_interface.awid.write(_wr_req_id_count.read());
                    aximm_interface.awaddr.write(dst_addr);
                    aximm_interface.awlen.write(0); // burst len of 1
                    aximm_interface.awsize.write(0); // burst len of 1
                    aximm_interface.awburst.write(0); // burst len of 1
                    aximm_interface.awuser.write(cur_wr_mem_req.src_port); // assuming the aw user field specifies port where the request came from?
                    // set unused transaction signals to invalid
                    aximm_interface.wvalid.write(false);
                    // aximm_interface.bready.write(false);

                    // aximm_interface.arvalid.write(false);
                } else {
                    _aximm_wr_state.write(AXIMM_WR_IDLE);
                    // aximm_interface.awvalid.write(false); // deassert addr valid if not sending a transaction , stay in AXIMM_RD_IDLE
                }
                break;
            }
            case AXIMM_WR_ADDR:
            {
                // Data
                _aximm_wr_state.write(AXIMM_WR_DATA_IPR);
                std::cout << "Sent WR Trans id " << _wr_req_id_count.read() << " DATA " << " @ Cycle " <<  GetSimulationCycle(5.0) << "!" << std::endl;
            
                cur_wr_mem_req = _mem_req_wr_ififo.fifo.front();
                _last_mem_wr_req = cur_wr_mem_req;

                aximm_interface.wdata.write(cur_wr_mem_req.wr_data);
                aximm_interface.wvalid.write(true);
                aximm_interface.wid.write(_wr_data_id_count.read());
                aximm_interface.wlast.write(true);
                aximm_interface.wuser.write(cur_wr_mem_req.src_port); // TODO figure out what the user ports
                // aximm_interface.bready.write(true); // tell AXI that we're ready for mem resp?
                // set unused transaction signals to invalid
                aximm_interface.awvalid.write(false);
                // aximm_interface.arvalid.write(false);
                _aximm_wr_num_sent_flits.write(_aximm_wr_num_sent_flits.read() + 1);

                _aximm_wr_tx_done.write(false);
                break;
            }
            case AXIMM_WR_DATA_IPR:
            {
                // Write Response Channel, if flit transaction accepted
                if (aximm_interface.wready.read() && aximm_interface.wvalid.read()) {
                    assert (!_mem_req_wr_ififo.empty); // make sure input fifo is not empty
                    aximm_interface.wvalid.write(false); // deassert data valid signal
                    // TODO add some logic to see if this is the last flit of the burst, if it is then move state to commit
                    _aximm_wr_state.write(AXIMM_WR_COMMIT); // Move state to commit
                    std::cout << module_name << ": Received WRITE flit response "
                        << "@ Cycle " <<  GetSimulationCycle(5.0) << "!"
                        << std::endl;
                    // Instructions are capable of single flit transmission(?) If so then we can pop fifos here and drop instruction info
                    _mem_req_wr_ififo.fifo.pop();
                    
                    // increment transaction ids
                    _wr_data_id_count.write(_wr_data_id_count.read() + 1);
                    _wr_req_id_count.write(_wr_req_id_count.read() + 1);
                } else {
                    // If not ready stay in AXIMM_WR_DATA_IPR
                    _aximm_wr_state.write(AXIMM_WR_DATA_IPR);
                }
                break;
            }
            case AXIMM_WR_COMMIT:
            {
                // Write response channel if entire burst has been accepted
                if (aximm_interface.bvalid.read() && aximm_interface.bready.read()) {
                    std::cout << module_name << ": Received WRITE burst response "
                        << "@ Cycle " <<  GetSimulationCycle(5.0) << "!"
                        << std::endl;
                    // If the WR Transaction BRESP value is not OKAY (00)
                    if (aximm_interface.bresp.read() != 0) {
                        std::cout << module_name << ": ERROR: WRITE burst response "
                            << "with response code " << aximm_interface.bresp.read()
                            << " @ Cycle " <<  GetSimulationCycle(5.0) << "!"
                            << std::endl;
                        wr_req_data_rdy.write(false);
                    } else {
                        wr_req_data_rdy.write(true);
                        wr_req_data.write(_last_mem_wr_req.wr_data);
                        // If WR Transaction was performed successfully
                        _aximm_wr_state.write(AXIMM_WR_IDLE); // Go back to idle state
                    }
                } else {
                    wr_req_data_rdy.write(false);
                    // If not ready stay in AXIMM_WR_COMMIT
                    _aximm_wr_state.write(AXIMM_WR_COMMIT);
                }
            }
            default:
                break;       
        }
        std::cout << "bvalid @ " << GetSimulationCycle(5.0) << ": " << aximm_interface.bvalid.read() << std::endl;

        // make sure input fifos are not empty
        /*
        bool no_mem_req_ififos_empty = std::find_if(_mem_req_ififos_empty.begin(), _mem_req_ififos_empty.end(), [](bool b) {return b == true;}) == _mem_req_ififos_empty.end();
        if (no_mem_req_ififos_empty){
            // Send AXI-MM read/write requests to the NoC        
            uint64_t mem_req_src_port = _src_port_fifo.front();
            uint64_t mem_req_dst_addr = _dst_port_fifo.front() + _target_addr_fifo.front();
            size_t mem_req_wr_data = _wr_data_fifo.front();
            bool mem_req_write_en = _wr_en_fifo.front();

            // unsigned int mem_req_target_channel = _target_channel_fifo.front(); // not using channel at the moment TODO when having single mem ctrl router for multiple channels
            // burst_len = _burst_len_fifo.front();
            // burst_len = _burst_len_fifo.front();

            if (mem_req_write_en) {
                // AXI-MM Write
                switch (_aximm_wr_state.read()){
                    case AXIMM_WR_IDLE:
                        // Address + Control 
                        _aximm_wr_state.write(AXIMM_WR_ADDR);
                        std::cout << "Sent WR Trans id " << _wr_req_id_count.read() << " ADDR + CTRL " << " @ Cycle " <<  GetSimulationCycle(5.0) << "!" << std::endl;

                        aximm_interface.awvalid.write(true);
                        aximm_interface.awid.write(_wr_req_id_count.read());
                        aximm_interface.awaddr.write(mem_req_dst_addr);
                        aximm_interface.awlen.write(0); // burst len of 1
                        aximm_interface.awsize.write(0); // burst len of 1
                        aximm_interface.awburst.write(0); // burst len of 1
                        aximm_interface.awuser.write(mem_req_src_port); // assuming the aw user field specifies port where the request came from?
                        // set unused transaction signals to invalid
                        aximm_interface.wvalid.write(false);
                        // aximm_interface.arvalid.write(false);

                        break;
                    case AXIMM_WR_ADDR:
                        // Data
                        _aximm_wr_state.write(AXIMM_WR_DATA_IPR);
                        std::cout << "Sent WR Trans id " << _wr_req_id_count.read() << " DATA " << " @ Cycle " <<  GetSimulationCycle(5.0) << "!" << std::endl;
                    
                        aximm_interface.wdata.write(mem_req_wr_data);
                        aximm_interface.wvalid.write(true);
                        aximm_interface.wid.write(_wr_data_id_count.read());
                        aximm_interface.wlast.write(true);
                        aximm_interface.wuser.write(mem_req_src_port); // TODO figure out what the user ports
                        // aximm_interface.bready.write(true); // tell AXI that we're ready for mem resp?
                        // set unused transaction signals to invalid
                        aximm_interface.awvalid.write(false);
                        // aximm_interface.arvalid.write(false);
                        _aximm_wr_num_sent_flits.write(_aximm_wr_num_sent_flits.read() + 1);

                        _aximm_wr_tx_done.write(false);
                        break;
                    default:
                        // do nothing if in any other case
                        break;                         
                }
            } else {
                
                switch (_aximm_rd_state.read()){
                    case AXIMM_RD_IDLE:
                        _aximm_rd_state.write(AXIMM_RD_ADDR);
                        std::cout << "Sent RD Trans id " << _rd_req_id_count.read() << " ADDR + CTRL" << " @ Cycle " <<  GetSimulationCycle(5.0) << "!" << std::endl;
                        
                        // AXI-MM Read
                        // Address + Control Channel Ready
                        aximm_interface.arvalid.write(true);
                        aximm_interface.arid.write(_rd_req_id_count.read());
                        aximm_interface.araddr.write(mem_req_dst_addr);
                        aximm_interface.arlen.write(0); // burst len of 1
                        aximm_interface.arsize.write(0); // burst len of 1
                        aximm_interface.arburst.write(0); // burst len of 1
                        aximm_interface.aruser.write(mem_req_src_port); // assuming the aw user field specifies port where the request came from?
                        
                        // aximm_interface.wvalid.write(false);
                        // aximm_interface.awvalid.write(false);
                        // _aximm_rd_tx_done.write(false);
                        break;
                    default:
                        break;
                }
            }
        } 

        // Do processes independant of dispatching transactions from instruction fifo
        
        // READ STATE
        switch (_aximm_rd_state.read()){
            case AXIMM_RD_ADDR:
                // If the Address + Control Transaction was recieved
                if (aximm_interface.arvalid.read() && aximm_interface.arready.read() && no_mem_req_ififos_empty){
                    aximm_interface.arvalid.write(false); // deassert address read valid
                    // Move back to idle state and deassert address read valid
                    _aximm_rd_state.write(AXIMM_RD_IDLE);
                    std::cout << "Sent AR transaction @ id: " << _rd_req_id_count.read() << std::endl;
                    _target_addr_fifo.pop();
                    _target_channel_fifo.pop();
                    _wr_en_fifo.pop();
                    _wr_data_fifo.pop();
                    _src_port_fifo.pop();
                    _dst_port_fifo.pop();
                    _rd_req_id_count.write(_rd_req_id_count.read() + 1);
                    // _aximm_rd_tx_done.write(true);
                }   
                break;
            default:
                break;
        }
        // WRITE STATE
        switch (_aximm_wr_state.read()){
            case AXIMM_WR_DATA_IPR:
                // Write Response Channel, if flit transaction accepted
                if (aximm_interface.wready.read() && aximm_interface.wvalid.read() && no_mem_req_ififos_empty) {
                    aximm_interface.wvalid.write(false); // deassert data valid signal
                    // TODO add some logic to see if this is the last flit of the burst, if it is then move state to commit
                    _aximm_wr_state.write(AXIMM_WR_COMMIT); // Move state to commit
                    std::cout << module_name << ": Received WRITE flit response "
                        << "@ Cycle " <<  GetSimulationCycle(5.0) << "!"
                        << std::endl;
                    // Instructions are capable of single flit transmission(?) If so then we can pop fifos here and drop instruction info
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
                break;
            case AXIMM_WR_COMMIT:
                // Write response channel if entire burst has been accepted
                if (aximm_interface.bvalid.read() && aximm_interface.bready.read()) {
                    std::cout << module_name << ": Received WRITE burst response "
                        << "@ Cycle " <<  GetSimulationCycle(5.0) << "!"
                        << std::endl;
                    // If the WR Transaction BRESP value is not OKAY (00)
                    if (aximm_interface.bresp.read() != 0) {
                        std::cout << module_name << ": ERROR: WRITE burst response "
                            << "with response code " << aximm_interface.bresp.read()
                            << " @ Cycle " <<  GetSimulationCycle(5.0) << "!"
                            << std::endl;
                    } else {
                        // If WR Transaction was performed successfully
                        _aximm_wr_state.write(AXIMM_WR_IDLE); // Go back to idle state
                    }
                }
                
            default:
                break;
        }
        */


        
        if (aximm_interface.bvalid.read() && aximm_interface.bready.read()) {
            std::cout << module_name << ": NON STATE MACHINE: Received WRITE burst response "
                << "@ Cycle " <<  GetSimulationCycle(5.0) << "!"
                << std::endl;
            // If the WR Transaction BRESP value is not OKAY (00)
            if (aximm_interface.bresp.read() != 0) {
                std::cout << module_name << ": ERROR: WRITE burst response "
                    << "with response code " << aximm_interface.bresp.read()
                    << " @ Cycle " <<  GetSimulationCycle(5.0) << "!"
                    << std::endl;
            } else {
                // If WR Transaction was performed successfully
                // _aximm_wr_state.write(AXIMM_WR_IDLE); // Go back to idle state
            }
        }
        /*
            // Write Response Channel , if transaction accepted
            if (aximm_interface.wready.read() && aximm_interface.wvalid.read() && no_mem_req_ififos_empty) {
                std::cout << module_name << ": Received WRITE flit response "
                    << "@ Cycle " <<  GetSimulationCycle(5.0) << "!"
                    << std::endl;
                // aximm_interface.bready.write(false);
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
        */


        // If transaction accepted and data channel ready
        if (aximm_interface.rvalid.read() && aximm_interface.rready.read()) {
            std::cout << module_name << ": Received READ response "
                        << "@ Cycle " <<  GetSimulationCycle(5.0) << "!" <<  ": "
                        << aximm_interface.rdata.read()
                        << std::endl;
            // verif signal
            sc_bv<AXI4_MAX_DATAW> rdata_bv_rd_req = aximm_interface.rdata.read();
            data_vector<uint64_t> rdata_rd_req(8); // 512 / 64
            aximm_512_bv_to_64_data_vector(rdata_bv_rd_req, rdata_rd_req, 64, 8);
            rd_req_data.write(rdata_rd_req[0]);
            rd_req_data_rdy.write(true);
        
            // put the returned value from rd transaction into input fifo
            if (_rd_data_input_fifo.size() < _fifos_depth){
                sc_bv<AXI4_MAX_DATAW> rdata_bv = aximm_interface.rdata.read();
                data_vector<int16_t> rdata(_num_input_elements);
                aximm_bv_to_data_vector(rdata_bv, rdata, _bitwidth, _num_input_elements);
                _rd_data_input_fifo.push(rdata);
            }
        } else {
            rd_req_data_rdy.write(false);
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
            std::string dest_name = "output_collector_0_inst.axis_slave__0";
            axis_interface.tdest.write(radsim_design.GetPortDestinationID(dest_name));
        }
        else {
            axis_interface.tvalid.write(false); // if no data in fifo, set valid to false
        }

        // Set Instruction FIFO full / empty signals
        _mem_req_rd_ififo.empty.write(_mem_req_rd_ififo.fifo.empty());
        _mem_req_wr_ififo.empty.write(_mem_req_wr_ififo.fifo.empty());
        _mem_req_rd_ififo.full.write(_mem_req_rd_ififo.fifo.size() >= _fifos_depth - 4);
        _mem_req_wr_ififo.full.write(_mem_req_wr_ififo.fifo.size() >= _fifos_depth - 4);


        // for (int i = 0; i < _mem_req_ififos_full.size(); i++) {
        //     _mem_req_ififos_full[i].write(_target_addr_fifo.size() >= _fifos_depth - 4);
        //     _mem_req_ififos_empty[i].write(_target_addr_fifo.empty());
        // }

        // Set I/O FIFO signals
        // Margin of 4 for input fifos and 2 for outputs not sure why
        _rd_data_ififo_empty.write(_rd_data_input_fifo.empty());
        _rd_data_ififo_full.write(_rd_data_input_fifo.size() >= _fifos_depth - 4); 
        _ofifo_empty.write(_output_fifo.empty());
        _ofifo_full.write(_output_fifo.size() >= _fifos_depth - 2);

        // received_responses.write(_num_received_responses);
        wait();

        // Pop from data mem_req input fifos and push 

    }
}

void black_box::RegisterModuleInfo() {
    _num_noc_axis_slave_ports = 0;
    _num_noc_axis_master_ports = 0;
    _num_noc_aximm_slave_ports = 0;
    _num_noc_aximm_master_ports = 0;
}


void black_box::RegisterModuleInfo(hw_module &module_conf) {
    _num_noc_axis_slave_ports = 0;
    _num_noc_axis_master_ports = 0;
    _num_noc_aximm_slave_ports = 0;
    _num_noc_aximm_master_ports = 0;

    // Register ports in module
    // ONLY WORKS FOR MANUALLY DEFINED PORTS TODO
    for (auto &port: module_conf.ports){
        if (port.type == "aximm") {
            if (port.is_master) {
                RegisterAximmMasterPort(port.name, &aximm_interface, _dataw);
            }
        }
        else if (port.type == "axis") {
            if (port.is_master) {
                RegisterAxisMasterPort(port.name, &axis_interface, _dataw, 0);
            }
        }
    }
    // aximm_master_ifs.resize(_num_noc_aximm_master_ports);
    // aximm_slave_ifs.resize(_num_noc_aximm_slave_ports);
    // axis_master_ifs.resize(_num_noc_axis_master_ports);
    // axis_slave_ifs.resize(_num_noc_axis_slave_ports);


    /*
        port_name = module_name + ".aximm_interface";
        RegisterAximmMasterPort(port_name, &aximm_interface, _dataw);

        port_name = module_name + ".axis_interface";
        RegisterAxisMasterPort(port_name, &axis_interface, _dataw, 0);
    */


  // for (unsigned int ch_id = 0; ch_id < _num_mem_channels; ch_id++) {
  //   port_name = module_name + ".aximm_interface_" + std::to_string(ch_id);
  //   RegisterAximmMasterPort(port_name, &aximm_interface[ch_id], _dataw);
  // }

  // for (unsigned int ch_id = 0; ch_id < _num_output_channels; ch_id++) {
  //   port_name = module_name + ".axis_interface_" + std::to_string(ch_id);
  //   RegisterAxisMasterPort(port_name, &axis_interface[ch_id], DATAW, 0);
  // }
}



