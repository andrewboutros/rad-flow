#include <radsim_inter_rad.hpp>

int num_wait = 1; //7;

std::ostream& operator<<(std::ostream& os, const axis_fields& I) {
    return os; //needed to create sc_fifo of custom struct type
}

RADSimInterRad::RADSimInterRad(const sc_module_name &name, sc_clock *inter_rad_clk, RADSimCluster* cluster) : sc_module(name) {
    this->cluster = cluster;
    this->clk(*inter_rad_clk);
    num_rads = cluster->num_rads;
    all_signals.init(num_rads + 1);
    std::cout << "num_rads is " << num_rads << std::endl;

    fifos_latency_counters.resize(num_rads);
    //std::cout << "fifos_latency_counters[0].size() " << fifos_latency_counters[0].size() << std::endl;

    for (int v = 0; v < num_rads; v++) { //width of vector = num of rads bc want fifo per rad
        sc_fifo<axis_fields>* new_fifo_ptr = new sc_fifo<axis_fields>(NUM_SLOTS);
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
        //for rising edge detection
        prev_valid.push_back(0);
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
        int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));

        //iterate thru all RADs
        for (int i = 0; i < num_rads; i++) {
            if (bw_counter >= bw_limit) {
                all_axis_slave_ports[i]->tready.write(true);
            }
            else {
                all_axis_slave_ports[i]->tready.write(false);
            }

            struct axis_fields curr_transaction;
            curr_transaction.tdata = all_axis_slave_ports[i]->tdata.read(); //0 bc adder
            curr_transaction.tuser = all_axis_slave_ports[i]->tuser.read();
            curr_transaction.tvalid = all_axis_slave_ports[i]->tvalid.read();
            curr_transaction.tlast = all_axis_slave_ports[i]->tlast.read();
            //all_axis_slave_ports[i]->tready.write(true);
            /*if (all_axis_slave_ports[i]->tready.read()) {
                //std::cout << "valid" << std::endl;
            }*/
            if (curr_transaction.tvalid && all_axis_slave_ports[i]->tready.read()) { //&& !prev_valid[i]) { //detect rising edge bc operating at higher clk freq than modules
                int dest_rad = curr_transaction.tuser.to_int64();
                //std::cout << dest_rad << std::endl;
                if (this->fifos[dest_rad]->nb_write(curr_transaction) != false) { //there was an available slot to write to
                    std::cout << "inter_rad fifo data WRITTEN on cycle " << curr_cycle << " is " << curr_transaction.tdata.to_uint64() << std::endl;
                    fifos_latency_counters[dest_rad].push_back(0); //for latency counters
                }
                //all_axis_slave_ports[i]->tready.write(false);
            }
            /*else if (!all_axis_slave_ports[i]->tready.read()) {
                if (bw_counter >= bw_limit) {
                    all_axis_slave_ports[i]->tready.write(true);
                    bw_counter = 0;
                }
                else {
                    bw_counter++;
                }
            }*/
            prev_valid[i] = curr_transaction.tvalid;
        }
        //wait(num_wait, SC_NS); //SC_NS); //eventually change to 1.3, SC_US -- assuming 2.6 us / 2 latency for one piece of data
        if (bw_counter >= bw_limit) {
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
        TODO: use tdest instead of tuser
        TODO: automating adding all fields to curr_transaction
        currently hardcoded to pull from same fifo that we use in writeFifo
    */
    while (true) {
        //std::cout << "inter_rad fifo free before READ is " << this->fifos[0]->num_free() << "/" << this->fifos[0]->num_available() << std::endl;
        
        //get current cycle for experiments
        int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));

        //sc_bv<DATAW> val = this->fifos[0]->read();
        for (int i = 0; i < num_rads; i++) { //iterate through all rad's fifos
            //increment delay on all counters
            for (int j = 0; j < fifos_latency_counters[i].size(); j++) {
                //std::cout << "i " << i << " j " << j << std::endl;
                fifos_latency_counters[i][j]++;
            }
            //try reading from front of fifo
            if ((this->fifos[i]->num_available() != 0) && (fifos_latency_counters[i][0] >= target_delay)){ //check that fifo is not empty
                //counter_delay = 0; //reset counter
                fifos_latency_counters[i].erase(fifos_latency_counters[i].begin()); //to reset counter, remove first elem
                struct axis_fields read_from_fifo;
                this->fifos[i]->nb_read(read_from_fifo);
                sc_bv<DATAW> val = read_from_fifo.tdata;
                int dest_device = read_from_fifo.tuser.to_uint64(); //#define AXIS_USERW     66
                
                //std::cout << "inter_rad fifo data READ is " << this->fifos[0]->read() << std::endl;
                if (read_from_fifo.tvalid) {
                    std::cout << "inter_rad fifo data READ is " << val.to_uint64() << " on cycle " << curr_cycle << std::endl;
                    //std::cout << "dest_device: " << dest_device << std::endl;
                    //all_signals[1].write(val); //works but replacing with axi
                    //all_axis_master_ports[1]->tdata.write(val); //1 bc sending to mult design
                    all_axis_master_signals[dest_device]->tdata.write(val); //works if write to either this or line above
                    all_axis_master_signals[dest_device]->tvalid.write(read_from_fifo.tvalid);
                    all_axis_master_signals[dest_device]->tlast.write(read_from_fifo.tlast);
                    //std::cout << "inter_rad fifo free after READ is " << this->fifos[0]->num_free() << "/" << this->fifos[0]->num_available() << std::endl;
                }
                else {
                    //no data to be written to any RAD's portal module
                    //all_axis_master_signals[0]->tvalid.write(false);
                    all_axis_master_signals[i]->tvalid.write(false);
                }
            
            }
            else {
                //no data to be written to any RAD's portal module
                //all_axis_master_signals[0]->tvalid.write(false);
                all_axis_master_signals[i]->tvalid.write(false);
            }
            
        }

        //wait(num_wait, SC_NS); //eventually change to 1.3, SC_US -- assuming 2.6 us / 2 latency for one piece of data
        wait();
        //wait(2);
    }
}