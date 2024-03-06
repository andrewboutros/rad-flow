#include <radsim_inter_rad.hpp>

std::ostream& operator<<(std::ostream& os, const axis_fields& I) {
    return os; //needed to create sc_fifo of custom struct type
}

RADSimInterRad::RADSimInterRad(const sc_module_name &name, sc_clock *inter_rad_clk, RADSimCluster* cluster) : sc_module(name) {
    this->cluster = cluster;
    this->clk(*inter_rad_clk);
    all_signals.init(cluster->num_rads + 1);

    for (int v = 0; v < cluster->num_rads; v++) { //width of vector = num of rads bc want fifo per rad
        sc_fifo<axis_fields>* new_fifo_ptr = new sc_fifo<axis_fields>(NUM_SLOTS);
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
    }
    SC_THREAD(writeFifo);
    SC_THREAD(readFifo);

}

RADSimInterRad::~RADSimInterRad() {
    int v;
    for (v = 0; v < this->cluster->num_rads; v++) {
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
    cluster->all_systems[i]->design_dut_inst->portal_in(all_signals[0]);
    //cluster->all_systems[i]->design_dut_inst->portal_out(all_signals[1]); //commenting out to demo sending data through fifo instead
    cluster->all_systems[i]->design_dut_inst->portal_out(all_signals[2]);
	cluster->all_systems[j]->design_dut_inst->portal_in(all_signals[1]);
	cluster->all_systems[j]->design_dut_inst->portal_out(all_signals[0]);
}

void
RADSimInterRad::ConnectRadAxi(int i) {
    all_axis_master_signals[i]->Connect(*(all_axis_master_ports[i]), cluster->all_systems[i]->design_dut_inst->design_top_portal_axis_slave); //Connect(axis_master_port &m, axis_slave_port &s)
    all_axis_slave_signals[i]->Connect(cluster->all_systems[i]->design_dut_inst->design_top_portal_axis_master, *(all_axis_slave_ports[i])); //Connect(axis_master_port &m, axis_slave_port &s)
}

bool wrote_yet = false;
void
RADSimInterRad::writeFifo() {
    // Always @ positive edge of the clock
    //wait();
    while (true) {
        //testing fifo
        //std::cout << "reached inter_rad " << std::endl;
        //sc_bv<DATAW> curr_val = cluster->all_systems[0]->design_dut_inst->portal_out.read(); //this works, but try using signal instead
        //sc_bv<DATAW> curr_val = all_signals[2]; //works but replacing with axi
        struct axis_fields curr_transaction;
        curr_transaction.tdata = all_axis_slave_ports[0]->tdata.read(); //0 bc adder
        curr_transaction.tuser = all_axis_slave_ports[0]->tuser.read();
        //std::cout << "inter_rad fifo free before write is " << this->fifos[0]->num_free() << "/" << this->fifos[0]->num_available() << std::endl;
        //if ((curr_val != 0) && (!wrote_yet)) {
        if ((curr_transaction.tdata != 0) && (!wrote_yet)) {
            if (this->fifos[0]->nb_write(curr_transaction) != false) { //there was an available slot to write to
                //std::cout << "inter_rad fifo free after write is " << this->fifos[0]->num_free() << "/" << this->fifos[0]->num_available() << std::endl;
                wrote_yet = true;
            }
        }
        wait(1, SC_NS); //eventually change to 1.3, SC_US -- assuming 2.6 us / 2 latency for one piece of data
    }
}

void
RADSimInterRad::readFifo() {
    // Always @ positive edge of the clock
    while (true) {
        //std::cout << "inter_rad fifo free before READ is " << this->fifos[0]->num_free() << "/" << this->fifos[0]->num_available() << std::endl;
        
        //sc_bv<DATAW> val = this->fifos[0]->read();

        struct axis_fields read_from_fifo;
        this->fifos[0]->nb_read(read_from_fifo);
        sc_bv<DATAW> val = read_from_fifo.tdata;
        int dest_device = read_from_fifo.tuser.to_int64(); //#define AXIS_USERW     66
        std::cout << "dest_device: " << dest_device << std::endl;

        //std::cout << "inter_rad fifo data READ is " << this->fifos[0]->read() << std::endl;
        if (val != false) {
            //std::cout << "inter_rad fifo data READ is " << val << std::endl;
            //all_signals[1].write(val); //works but replacing with axi
            //all_axis_master_ports[1]->tdata.write(val); //1 bc sending to mult design
            all_axis_master_signals[dest_device]->tdata.write(val); //works if write to either this or line above
            //std::cout << "inter_rad fifo free after READ is " << this->fifos[0]->num_free() << "/" << this->fifos[0]->num_available() << std::endl;
        }
        std::cout << "radsim_inter_rad value is (val): " << val << std::endl;
        /*std::cout << "radsim_inter_rad value is (master_ports): " << all_axis_master_ports[1]->tdata.read() << std::endl;
        std::cout << "radsim_inter_rad value is (master_signals): " << all_axis_master_signals[1]->tdata.read() << std::endl;
        std::cout << "radsim_inter_rad value is (dut_inst): " << cluster->all_systems[1]->design_dut_inst->design_top_portal_axis_slave.tdata.read() << std::endl;
        */

        wait(1, SC_NS); //eventually change to 1.3, SC_US -- assuming 2.6 us / 2 latency for one piece of data
    }
}