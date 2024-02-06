#include <portal.hpp>

portal::portal(const sc_module_name &name, RADSimDesignContext* radsim_design) //AKB added last arg
    : RADSimModule(name, radsim_design) {

    //maybe add combinational logic if applicable later
    SC_CTHREAD(Tick, clk.pos());
    // This function must be defined & called for any RAD-Sim module to register
    // its info for automatically connecting to the NoC
    this->RegisterModuleInfo(); //can comment out if not connecting to NoC
}


portal::~portal() {}

//void portal::Assign() { //combinational logic
    //maybe add reset signal later
//}

bool counter = 0;
void portal::Tick() { //sequential logic
    portal_out.write(counter);
    wait();
    //Always @ positive edge of clock
    while (true) {
        if (counter == 0) {
            counter = 1;
        }
        else {
            counter = 0;
        }
        portal_out.write(counter);
        //std::cout << module_name << ": Wire in is showing " << portal_in.read() << std::endl;
        //std::cout << counter << std::endl;
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

    _num_noc_axis_slave_ports = 0;
    _num_noc_axis_master_ports = 0;
    _num_noc_aximm_slave_ports = 0;
    _num_noc_aximm_master_ports = 0;

    port_name = module_name + ".axis_add_portal_master_interface";
    RegisterAxisMasterPort(port_name, &axis_add_portal_master_interface, DATAW, 0);

}