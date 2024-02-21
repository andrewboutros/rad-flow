#include <radsim_inter_rad.hpp>

RADSimInterRad::RADSimInterRad(const sc_module_name &name, sc_clock *inter_rad_clk, RADSimCluster* cluster) : sc_module(name) {
    this->cluster = cluster;
    this->clk(*inter_rad_clk);
    all_signals.init(cluster->num_rads + 1);

    for (int v = 0; v < cluster->num_rads; v++) { //width of vector = num of rads bc want fifo per rad
        sc_fifo<sc_bv<DATAW>>* new_fifo_ptr = new sc_fifo<sc_bv<DATAW>>(NUM_SLOTS);
        fifos.push_back(new_fifo_ptr);
    }

    SC_THREAD(writeFifo);
    SC_THREAD(readFifo);

}

RADSimInterRad::~RADSimInterRad() {
    for (int v = 0; v < this->cluster->num_rads; v++) {
        delete fifos[v];
    }
}

void
RADSimInterRad::ConnectRadPair(int i, int j) {
    //this works, commenting out to try manually writing to port
    cluster->all_systems[i]->design_dut_inst->portal_in(all_signals[0]);
    //cluster->all_systems[i]->design_dut_inst->portal_out(all_signals[1]);
    cluster->all_systems[i]->design_dut_inst->portal_out(all_signals[2]); //temp added extra signal just to bind
	cluster->all_systems[j]->design_dut_inst->portal_in(all_signals[1]);
	cluster->all_systems[j]->design_dut_inst->portal_out(all_signals[0]);
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
        sc_bv<DATAW> curr_val = all_signals[2];
        std::cout << "inter_rad fifo free before write is " << this->fifos[0]->num_free() << "/" << this->fifos[0]->num_available() << std::endl;
        if ((curr_val != 0) && (!wrote_yet)) {
            //this->fifos[0]->nb_write(curr_val);
            if (this->fifos[0]->nb_write(curr_val) != false) { //there was an available slot to write to
                std::cout << "inter_rad fifo free after write is " << this->fifos[0]->num_free() << "/" << this->fifos[0]->num_available() << std::endl;
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
        std::cout << "inter_rad fifo free before READ is " << this->fifos[0]->num_free() << "/" << this->fifos[0]->num_available() << std::endl;
        
        //sc_bv<DATAW> val = this->fifos[0]->read();

        sc_bv<DATAW> val;
        this->fifos[0]->nb_read(val);

        //std::cout << "inter_rad fifo data READ is " << this->fifos[0]->read() << std::endl;
        if (val != false) {
            std::cout << "inter_rad fifo data READ is " << val << std::endl;
            all_signals[1].write(val);
            std::cout << "inter_rad fifo free after READ is " << this->fifos[0]->num_free() << "/" << this->fifos[0]->num_available() << std::endl;
        }
        
        wait(1, SC_NS); //eventually change to 1.3, SC_US -- assuming 2.6 us / 2 latency for one piece of data
    }
}