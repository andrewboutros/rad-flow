#include <portal.hpp>

portal::portal(const sc_module_name &name, RADSimDesignContext* radsim_design)
    : RADSimModule(name, radsim_design) {

    this->radsim_design = radsim_design;

    //combinational logic
    SC_METHOD(Assign);
    //sequential logic
    SC_CTHREAD(Tick, clk.pos());
    // This function must be defined & called for any RAD-Sim module to register
    // its info for automatically connecting to the NoC
    this->RegisterModuleInfo(); //can comment out if not connecting to NoC
}


portal::~portal() {}

void portal::Assign() { //combinational logic
    //maybe add reset signal later
    axis_portal_slave_interface.tready.write(true);
}

int counter = 0;
sc_bv<DATAW> data_to_buffer = 0;
sc_bv<AXIS_USERW> dest_device = 1; //for testing, fixed at 1 to send to RAD1 which has mult design; //#define AXIS_USERW     66
//bool got_data = false;
void portal::Tick() { //sequential logic
    //portal_out.write(counter);
    //portal_out_axi.tdata.write(counter);
    portal_recvd.write(0);
    portal_axis_master.tvalid.write(false);
    //bool test_ready_toggle = false;
    wait();
    //Always @ positive edge of clock
    while (true) {

        if (axis_portal_slave_interface.tvalid.read() &&
            axis_portal_slave_interface.tready.read()) {
            //std::cout << "Also got here" << std:: endl;
            std::cout << "Add design sending data over portal module " << module_name << ": Got Transaction (user = "
                        << axis_portal_slave_interface.tuser.read().to_uint64() << ") (addend = "
                        << axis_portal_slave_interface.tdata.read().to_uint64() << ")!"
                        << std::endl;
             data_to_buffer = axis_portal_slave_interface.tdata.read();
             //got_data = true;
             portal_axis_fields curr_transaction = {
                axis_portal_slave_interface.tvalid.read(),
                axis_portal_slave_interface.tready.read(),
                axis_portal_slave_interface.tdata.read(),
                axis_portal_slave_interface.tstrb.read(),
                axis_portal_slave_interface.tkeep.read(),
                axis_portal_slave_interface.tlast.read(),
                axis_portal_slave_interface.tid.read(),
                axis_portal_slave_interface.tdest.read(),
                dest_device //tuser field
             };

            portal_axis_fifo.push(curr_transaction);
        }

        if ((portal_axis_fifo.size() > 0) ) { //&& test_ready_toggle) {
            portal_axis_fields curr_transaction = portal_axis_fifo.front();
            portal_axis_master.tdata.write(curr_transaction.tdata);
            portal_axis_master.tuser.write(dest_device);
            portal_axis_master.tvalid.write(true);
            portal_axis_master.tlast.write(curr_transaction.tlast);
            //test_ready_toggle = false;
        }
        else {
            //counter++;
            portal_axis_master.tdata.write(0);
            portal_axis_master.tuser.write(dest_device);
            portal_axis_master.tvalid.write(false);
            //test_ready_toggle = true;
        }

        /*if (portal_axis_master.tvalid.read()) {
            test_ready_toggle = !test_ready_toggle;
        }*/
        
        if (portal_axis_master.tvalid.read() && portal_axis_master.tready.read()) { // && test_ready_toggle) { 
            //pop out of fifo
            if (!portal_axis_fifo.empty()) {
                //test_ready_toggle = false;
                portal_axis_fifo.pop();
                std::cout << "portal.cpp in add design sent to dest_device: " << dest_device.to_int64() << std::endl;
                portal_recvd.write(1);
                if (portal_axis_master.tlast.read()) {
                    int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
                    std::cout << "Add design portal.cpp sent last data via inter_rad at cycle " << curr_cycle << std::endl;
                }
            }
            else { //should never reach here because valid should be false if fifo is empty
                std::cout << "reached here but why? portal_axis_fifo.size(): " << portal_axis_fifo.size() << std::endl;
            }
        }
        /*else if (!test_ready_toggle) {
            test_ready_toggle = true;
        }*/

        wait();
    }
}

void portal::RegisterModuleInfo() {
    std::string port_name;
    _num_noc_axis_slave_ports = 0;
    _num_noc_axis_master_ports = 0;
    _num_noc_aximm_slave_ports = 0;
    _num_noc_aximm_master_ports = 0;

    port_name = module_name + ".axis_portal_slave_interface";
    RegisterAxisSlavePort(port_name, &axis_portal_slave_interface, DATAW, 0);

    port_name = module_name + ".axis_portal_master_interface";
    RegisterAxisMasterPort(port_name, &axis_portal_master_interface, DATAW, 0);
}