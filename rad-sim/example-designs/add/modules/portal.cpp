#include <portal.hpp>

portal::portal(const sc_module_name &name, RADSimDesignContext* radsim_design) //AKB added last arg
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
    axis_add_portal_slave_interface.tready.write(true);
}

int counter = 0;
sc_bv<DATAW> data_to_buffer = 0;
sc_bv<AXIS_USERW> dest_device; //#define AXIS_USERW     66
bool got_data = false;
void portal::Tick() { //sequential logic
    portal_out.write(counter);
    //portal_out_axi.tdata.write(counter);
    portal_recvd.write(0);
    wait();
    //Always @ positive edge of clock
    while (true) {
        /*if (counter == 0) {
            counter = 1;
        }
        else {
            counter = 0;
        }
        portal_out.write(counter);*/
        //std::cout << module_name << ": Wire in is showing " << portal_in.read() << std::endl;
        //std::cout << counter << std::endl;

        /*std::cout << axis_add_portal_slave_interface.tvalid.read() << std::endl;
        std::cout << axis_add_portal_slave_interface.tready.read() << std::endl;*/
        if (axis_add_portal_slave_interface.tvalid.read() &&
            axis_add_portal_slave_interface.tready.read()) {
            std::cout << "Also got here" << std:: endl;
            std::cout << "Add design sending data over portal module " << module_name << ": Got Transaction (user = "
                        << axis_add_portal_slave_interface.tuser.read().to_uint64() << ") (addend = "
                        << axis_add_portal_slave_interface.tdata.read().to_uint64() << ")!"
                        << std::endl;
             data_to_buffer = axis_add_portal_slave_interface.tdata.read();
             dest_device = 1; //for testing, fixed at 1 to send to RAD1 which has mult design
             got_data = true;
        //}
        //if (got_data) {
            //std::cout << "counter : " << counter << std::endl;
            //if (counter == 3) {
            //if (counter == 0) { //always send, do not buffer in portal module bc moved that to interrad now
            //    counter = 0;
                //portal_out.write(data_to_buffer); //works but replace with axi
                portal_axis_master.tdata.write(data_to_buffer);
                portal_axis_master.tuser.write(dest_device);
                portal_axis_master.tvalid.write(true);
                portal_axis_master.tlast.write(axis_add_portal_slave_interface.tlast.read());
                std::cout << "portal.cpp in add design sent dest_device: " << dest_device.to_int64() << std::endl;
                portal_recvd.write(1);
                if (axis_add_portal_slave_interface.tlast.read()) {
                    int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
                    std::cout << "Add design portal.cpp sent last data via inter_rad at cycle " << curr_cycle << std::endl;
                }
            }
            else {
                //counter++;
                portal_axis_master.tvalid.write(false);
            }
        //}

        wait();
    }
}

void portal::RegisterModuleInfo() {
    //I don't think this is needed unless I add AXI Interface -- nvm, need bc is virtual fn in derived class
    //now adding AXI slave interface
    std::string port_name;
    _num_noc_axis_slave_ports = 0;
    _num_noc_axis_master_ports = 0;
    _num_noc_aximm_slave_ports = 0;
    _num_noc_aximm_master_ports = 0;

    port_name = module_name + ".axis_add_portal_slave_interface";
    //std::cout << port_name << std::endl;
    RegisterAxisSlavePort(port_name, &axis_add_portal_slave_interface, DATAW, 0);

    /*_num_noc_axis_slave_ports = 0;
    _num_noc_axis_master_ports = 0;
    _num_noc_aximm_slave_ports = 0;
    _num_noc_aximm_master_ports = 0;*/

    port_name = module_name + ".axis_add_portal_master_interface";
    RegisterAxisMasterPort(port_name, &axis_add_portal_master_interface, DATAW, 0);

    /*port_name = module_name + ".portal_axis_master";
    RegisterAxisMasterPort(port_name, &portal_axis_master, DATAW, 0);

    port_name = module_name + ".portal_axis_slave";
    RegisterAxisSlavePort(port_name, &portal_axis_slave, DATAW, 0);*/

}