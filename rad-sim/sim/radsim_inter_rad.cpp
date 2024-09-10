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
    //std::cout << "num_rads is " << num_rads << std::endl;

    fifos_latency_counters.resize(num_rads);
    //std::cout << "fifos_latency_counters[0].size() " << fifos_latency_counters[0].size() << std::endl;
    int inter_rad_fifo_num_slots = radsim_config.GetIntKnobShared("inter_rad_fifo_num_slots"); //1000;
    for (int v = 0; v < num_rads; v++) { //width of vector = num of rads bc want fifo per rad
        sc_fifo<axis_fields>* new_fifo_ptr = new sc_fifo<axis_fields>(inter_rad_fifo_num_slots);
        fifos.push_back(new_fifo_ptr);
        //adding to axi vectors
        axis_signal* new_axis_signal = new axis_signal;
        all_axis_master_signals.push_back(new_axis_signal);
        new_axis_signal = new axis_signal; //second signal (one for master, one for slave)
        all_axis_slave_signals.push_back(new_axis_signal);
        axis_slave_port* new_axis_slave_port = new axis_slave_port;
        //new_axis_slave_port->tready.write(false); //initialize ready to false
        all_axis_slave_ports.push_back(new_axis_slave_port);
        axis_master_port* new_axis_master_port = new axis_master_port;
        all_axis_master_ports.push_back(new_axis_master_port);
        std::cout << "RADSimInterRad: " << v << std::endl;
    }
    SC_CTHREAD(writeFifo, clk.pos());
    SC_CTHREAD(readFifo, clk.pos());

    //testing
    /*for (int i = 0; i < num_rads; i++) {
        for (int j = 0; j < NUM_SLOTS; j++) {
            std::cout << fifos_latency_counters[i][j] << std::endl;
        }
    }*/
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

void
RADSimInterRad::ConnectRadPair(int i, int j) {
    //this works
    /*cluster->all_systems[i]->design_dut_inst->portal_in(all_signals[0]);
    //cluster->all_systems[i]->design_dut_inst->portal_out(all_signals[1]); //commenting out to demo sending data through fifo instead
    cluster->all_systems[i]->design_dut_inst->portal_out(all_signals[2]);
	cluster->all_systems[j]->design_dut_inst->portal_in(all_signals[1]);
	cluster->all_systems[j]->design_dut_inst->portal_out(all_signals[0]);*/
}

void
RADSimInterRad::ConnectRadAxi(int i) {
    all_axis_master_signals[i]->Connect(*(all_axis_master_ports[i]), cluster->all_systems[i]->design_dut_inst->design_top_portal_axis_slave); //Connect(axis_master_port &m, axis_slave_port &s)
    all_axis_slave_signals[i]->Connect(cluster->all_systems[i]->design_dut_inst->design_top_portal_axis_master, *(all_axis_slave_ports[i])); //Connect(axis_master_port &m, axis_slave_port &s)
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
    //wait();
    for (int i = 0; i < num_rads; i++) {
        all_axis_slave_signals[i]->tready.write(true);
    }
    int bw_counter = 0;
    wait();
    while (true) {
        //get current cycle for experiments
        int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));

        //iterate thru all RADs
        for (int i = 0; i < num_rads; i++) {
            if (bw_counter < radsim_config.GetIntKnobShared("inter_rad_bw_accept_cycles")) { //>= bw_limit) {
                all_axis_slave_ports[i]->tready.write(true);
            }
            else {
                all_axis_slave_ports[i]->tready.write(false);
            }

            struct axis_fields curr_transaction;
            curr_transaction.tdata = all_axis_slave_ports[i]->tdata.read(); //0 bc adder
            curr_transaction.tuser = all_axis_slave_ports[i]->tuser.read();
            //std::cout << "curr_transaction.tuser in interrad: " << curr_transaction.tuser << std::endl;
            curr_transaction.tvalid = all_axis_slave_ports[i]->tvalid.read();
            curr_transaction.tlast = all_axis_slave_ports[i]->tlast.read();
            //since crossing RADs, DEST_LOCAL_NODE is now DEST_REMOTE_NODE, and DEST_REMOTE_NODE can be reset to 0
            sc_bv<AXIS_DESTW> concat_dest_swap = all_axis_slave_ports[i]->tdest.read();
            //std::cout << "concat_dest_swap: " << concat_dest_swap << std::endl;
            DEST_LOCAL_NODE(concat_dest_swap) = DEST_REMOTE_NODE(all_axis_slave_ports[i]->tdest.read());
            DEST_REMOTE_NODE(concat_dest_swap) = 0;
            curr_transaction.tdest = concat_dest_swap; //all_axis_slave_ports[i]->tdest.read();
            //std::cout << "curr_transaction.tdest: " << concat_dest_swap << std::endl;
            //all_axis_slave_ports[i]->tready.write(true);
            /*if (all_axis_slave_ports[i]->tready.read()) {
                //std::cout << "valid" << std::endl;
            }*/
            if (curr_transaction.tvalid && all_axis_slave_ports[i]->tready.read()) {
                unsigned int dest_rad = DEST_RAD(curr_transaction.tdest).to_uint64();
                //std::cout << "radsim_inter_rad.cpp dest_rad is: "<< dest_rad << std::endl;
                if (this->fifos[dest_rad]->nb_write(curr_transaction) != false) { //there was an available slot to write to
                    /* START FOR DEBUG */
                    // sc_bv<AXIS_MAX_DATAW> rx_tdata_bv = curr_transaction.tdata;
                    // data_vector<int16_t> rx_tdata(32); //NOTE: type needs to match what is supported in example-designs/{design}/modules/sim_utils.hpp
                    // bv_to_data_vector(rx_tdata_bv, rx_tdata, 32);
                    // std::cout << "inter_rad fifo data WRITTEN on cycle " << curr_cycle << " is " << rx_tdata << std::endl;
                    // std::cout << "inter_rad fifo data WRITTEN on cycle " << curr_cycle << " is " << curr_transaction.tdata.to_uint64() << std::endl;
                    /* END FOR DEBUG */
                    fifos_latency_counters[dest_rad].push_back(0); //for latency counters
                }
                else {
                    std::cout << "WRITE FIFO FULL: packet dropped at inter_rad: could not write into internal fifo. Packets dropped count: " << write_fifo_packet_drop_count << std::endl;
                    write_fifo_packet_drop_count++;
                }
            }

            // if (all_axis_slave_ports[i]->tready.read()) {
            //     std::cout << "June-27: inter_rad fifo is ready with counter " << bw_counter << std::endl;
            // }
            // else { //will see the ready signal one cycle after the counter var val changes bc is systemc signal 
            //     std::cout << "June-27: inter_rad fifo NOT ready with counter " << bw_counter << std::endl;
            // }

        }

        if (bw_counter >= (radsim_config.GetIntKnobShared("inter_rad_bw_total_cycles") - 1)) { //bw_limit) {
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
        DONE: use tdest instead of tuser
    */
    int target_delay = radsim_config.GetIntKnobShared("inter_rad_latency_cycles");
    while (true) {
        //std::cout << "inter_rad fifo free before READ is " << this->fifos[0]->num_free() << "/" << this->fifos[0]->num_available() << std::endl;
        
        //get current cycle for experiments
        int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));

        //sc_bv<AXIS_MAX_DATAW> val = this->fifos[0]->read();
        for (int i = 0; i < num_rads; i++) { //iterate through all rad's fifos
            //increment delay on all counters
            for (int j = 0; j < fifos_latency_counters[i].size(); j++) {
                //std::cout << "i " << i << " j " << j << std::endl;
                fifos_latency_counters[i][j]++;
            }
            //try reading from front of fifo
            //std::cout << "all_axis_master_signals[dest_device]->tready.read(): " << all_axis_master_signals[dest_device]->tready.read() << std::endl;
            //tried adding && (all_axis_master_signals[dest_device]->tready.read() as condn below, but no support for peek on sc_fifo to get the dest
            //TODO: replace sc_fifo with something else std::queue that can support peeks
            //IMPORTANT: currently does not accept backpressure. Portal module must create a buffer for backpressure on the RAD's NoC
            if ( (this->fifos[i]->num_available() != 0) && (fifos_latency_counters[i][0] >= target_delay) ){ //check that fifo is not empty
                fifos_latency_counters[i].erase(fifos_latency_counters[i].begin()); //to reset counter, remove first elem
                struct axis_fields read_from_fifo;
                this->fifos[i]->nb_read(read_from_fifo);
                sc_bv<AXIS_MAX_DATAW> val = read_from_fifo.tdata;
                int dest_device = (DEST_RAD(read_from_fifo.tdest)).to_uint64(); //#define AXIS_USERW     66
                
                //std::cout << "inter_rad fifo data READ is " << this->fifos[0]->read() << std::endl;
                if (read_from_fifo.tvalid) {
                    //std::cout << "inter_rad fifo data READ is " << val.to_uint64() << " on cycle " << curr_cycle << std::endl;
                    //std::cout << "dest_device: " << dest_device << std::endl;
                    //all_signals[1].write(val); //works but replacing with axi
                    all_axis_master_signals[dest_device]->tdata.write(val); //works if write to either this or line above
                    all_axis_master_signals[dest_device]->tvalid.write(read_from_fifo.tvalid);
                    all_axis_master_signals[dest_device]->tlast.write(read_from_fifo.tlast);
                    all_axis_master_signals[dest_device]->tdest.write(read_from_fifo.tdest);
                    all_axis_master_signals[dest_device]->tuser.write(read_from_fifo.tuser);
                    //std::cout << "inter_rad fifo free after READ is " << this->fifos[0]->num_free() << "/" << this->fifos[0]->num_available() << std::endl;
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
