#include <radsim_inter_rad.hpp>

std::ostream& operator<<(std::ostream& os, const axis_fields& I) {
    return os; //needed to create sc_fifo of custom struct type
}

RADSimInterRad::RADSimInterRad(const sc_module_name &name, sc_clock *inter_rad_clk, RADSimCluster* cluster) : sc_module(name) {
    std::cout << "RADSimInterRad AXIS_MAX_DATAW " << AXIS_MAX_DATAW << std::endl;
    this->cluster = cluster;
    this->clk(*inter_rad_clk);
    num_rads = cluster->num_rads;
    all_signals.init(num_rads + 1);

    fifos_latency_counters.resize(num_rads);
    int inter_rad_fifo_num_slots = radsim_config.GetIntKnobShared("inter_rad_fifo_num_slots");
    for (int v = 0; v < num_rads; v++) { //width of vector = num of rads bc want fifo per rad
        sc_fifo<axis_fields>* new_fifo_ptr = new sc_fifo<axis_fields>(inter_rad_fifo_num_slots);
        fifos.push_back(new_fifo_ptr);
        //adding to axi vectors
        axis_signal* new_axis_signal = new axis_signal;
        all_axis_master_signals.push_back(new_axis_signal);
        new_axis_signal = new axis_signal; //second signal (one for master, one for slave)
        all_axis_slave_signals.push_back(new_axis_signal);
        axis_slave_port* new_axis_slave_port = new axis_slave_port;
        all_axis_slave_ports.push_back(new_axis_slave_port);
        axis_master_port* new_axis_master_port = new axis_master_port;
        all_axis_master_ports.push_back(new_axis_master_port);
        std::cout << "RADSimInterRad: " << v << std::endl;
    }
    SC_CTHREAD(writeFifo, clk.pos());
    SC_CTHREAD(readFifo, clk.pos());

}

RADSimInterRad::~RADSimInterRad() {
    int v;
    for (v = 0; v < num_rads; v++) {
        delete fifos[v];
        delete all_axis_master_signals[v];
        delete all_axis_slave_signals[v];
        delete all_axis_slave_ports[v];
        delete all_axis_master_ports[v];
    }
}

//Connect the axi slave interface of each portal module to its corresponding RADSimInterRad axi master interface, and vice versa
void
RADSimInterRad::ConnectClusterInterfaces(int rad_id) {
    #ifndef SINGLE_RAD
    all_axis_master_signals[rad_id]->Connect(*(all_axis_master_ports[rad_id]), cluster->all_systems[rad_id]->design_dut_inst->design_top_portal_axis_slave);
    all_axis_slave_signals[rad_id]->Connect(cluster->all_systems[rad_id]->design_dut_inst->design_top_portal_axis_master, *(all_axis_slave_ports[rad_id]));
    #endif
}

bool wrote_yet = false;
int write_fifo_packet_drop_count = 0;
void
RADSimInterRad::writeFifo() {
    /*
    Always @ positive edge of the clock
    Writes into fifo from axi interface
    Iterates over all_axis_slave_ports entries, 
    Checks if the data is valid and only write into fifo then
    Writes into the fifo corresponding to the dest.
    TODO: use tdest instead of tuser
    TODO: automating adding all fields to curr_transaction
    */

    //initial setup
    for (int i = 0; i < num_rads; i++) {
        all_axis_slave_signals[i]->tready.write(true);
    }
    int bw_counter = 0; //counter used for bandwidth constraint on inter-rad network
    wait();
    
    //every clock cycle
    while (true) {
        //get current cycle for experiments
        //int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));

        //iterate thru all RADs
        for (int i = 0; i < num_rads; i++) {
            //from bandwidth constraint, we calculated how many cycles at a time we can accept data into the inter-rad network
            //when bw_counter exceeds those cycles, we can no longer accept more data
            //we assert backpressure accordingly to prevent accepting of data into the inter-rad.
            if (bw_counter < radsim_config.GetIntKnobShared("inter_rad_bw_accept_cycles")) {
                all_axis_slave_ports[i]->tready.write(true);
            }
            else {
                all_axis_slave_ports[i]->tready.write(false);
            }

            //Obtain relevant axi-s fields
            struct axis_fields curr_transaction;
            curr_transaction.tdata = all_axis_slave_ports[i]->tdata.read();
            curr_transaction.tuser = all_axis_slave_ports[i]->tuser.read();
            curr_transaction.tvalid = all_axis_slave_ports[i]->tvalid.read();
            curr_transaction.tlast = all_axis_slave_ports[i]->tlast.read();
            
            //Update the dest field because data will now move from an initial RAD to a different RAD
            //Once we reach the destination RAD, what was previously a remote NoC node destination is now local
            //DEST_LOCAL_NODE is now DEST_REMOTE_NODE, and DEST_REMOTE_NODE can be reset to 0
            sc_bv<AXIS_DESTW> concat_dest_swap = all_axis_slave_ports[i]->tdest.read();
            DEST_LOCAL_NODE(concat_dest_swap) = DEST_REMOTE_NODE(all_axis_slave_ports[i]->tdest.read());
            DEST_REMOTE_NODE(concat_dest_swap) = 0;
            curr_transaction.tdest = concat_dest_swap;

            if (curr_transaction.tvalid && all_axis_slave_ports[i]->tready.read()) {
                unsigned int dest_rad = DEST_RAD(curr_transaction.tdest).to_uint64();
                if (this->fifos[dest_rad]->nb_write(curr_transaction) != false) { //there was an available slot to write to
                    fifos_latency_counters[dest_rad].push_back(0); //for latency counters
                }
                else {
                    std::cout << "WRITE FIFO FULL: packet dropped at inter_rad: could not write into internal fifo. Packets dropped count: " << write_fifo_packet_drop_count << std::endl;
                    write_fifo_packet_drop_count++;
                }
            }
        }

        //Update bandwidth cycle counter. Reset to 0 if we have accepted data for enough cycles to meet our bandwidth limit.
        //Else, increment the cycle counter.
        if (bw_counter >= (radsim_config.GetIntKnobShared("inter_rad_bw_total_cycles") - 1)) {
            bw_counter = 0;
        }
        else {
            bw_counter++;
        }
        wait();
    }
}

void
RADSimInterRad::readFifo() {
    /* 
    Always @ positive edge of the clock
    Read from fifo slot
    Iterates thru all fifos
    Matches the dest index of fifo to the dest rad
    */
    int target_delay = radsim_config.GetIntKnobShared("inter_rad_latency_cycles");
    while (true) {
        
        //get current cycle for experiments
        //int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));

        for (int i = 0; i < num_rads; i++) { //iterate through all rad's fifos
            
            //increment delay on all counters
            for (unsigned int j = 0; j < fifos_latency_counters[i].size(); j++) {
                fifos_latency_counters[i][j]++;
            }
            
            //Try reading from front of fifo. sc_fifo does not support peek so this is required instead to obtain dest.
            //TODO: replace sc_fifo with something else e.g., std::queue that can support peeks
            //IMPORTANT: currently does not accept backpressure. Portal module must create a buffer for backpressure on the RAD's NoC
            if ( (this->fifos[i]->num_available() != 0) && (fifos_latency_counters[i][0] >= target_delay) ){ //check that fifo is not empty
                fifos_latency_counters[i].erase(fifos_latency_counters[i].begin()); //to reset counter, remove first elem
                struct axis_fields read_from_fifo;
                this->fifos[i]->nb_read(read_from_fifo);
                sc_bv<AXIS_MAX_DATAW> val = read_from_fifo.tdata;
                int dest_device = (DEST_RAD(read_from_fifo.tdest)).to_uint64();
                
                if (read_from_fifo.tvalid) {
                    all_axis_master_signals[dest_device]->tdata.write(val);
                    all_axis_master_signals[dest_device]->tvalid.write(read_from_fifo.tvalid);
                    all_axis_master_signals[dest_device]->tlast.write(read_from_fifo.tlast);
                    all_axis_master_signals[dest_device]->tdest.write(read_from_fifo.tdest);
                    all_axis_master_signals[dest_device]->tuser.write(read_from_fifo.tuser);
                }
                else {
                    //no data to be written to any RAD's portal module
                    all_axis_master_signals[i]->tvalid.write(false);
                }
            
            }
            else {
                //no data to be written to any RAD's portal module
                all_axis_master_signals[i]->tvalid.write(false);
            }
            
        }

        wait();

    }
}
