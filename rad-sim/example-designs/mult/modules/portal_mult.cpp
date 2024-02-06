#include <portal_mult.hpp>

portal_mult::portal_mult(const sc_module_name &name, RADSimDesignContext* radsim_design) //AKB added last arg
    : RADSimModule(name, radsim_design) {

    //maybe add combinational logic if applicable later
    SC_CTHREAD(Tick, clk.pos());
    this->RegisterModuleInfo(); //can comment out if not connecting to NoC
}


portal_mult::~portal_mult() {}

/*void portal::Assign() { //combinational logic
    //maybe add reset signal later
}*/

int counter_mult = 0;
void portal_mult::Tick() { //sequential logic
    portal_out.write(counter_mult);
    wait();
    //Always @ positive edge of clock
    while (true) {
        if (counter_mult == 0) {
            counter_mult = 1;
        }
        else {
            counter_mult = 0;
        }
        portal_out.write(counter_mult);
        //std::cout << module_name << ": Wire in is showing " << portal_in.read() << std::endl;
        //std::cout << counter << std::endl;
        wait();
    }
}

void portal_mult::RegisterModuleInfo() {
    //I don't think this is needed unless I add AXI Interface -- nvm, need bc is virtual fn in derived class
    std::string port_name;
    _num_noc_axis_slave_ports = 0;
    _num_noc_axis_master_ports = 0;
    _num_noc_aximm_slave_ports = 0;
    _num_noc_aximm_master_ports = 0;

    port_name = module_name + ".axis_mult_portal_slave_interface";
    //std::cout << port_name << std::endl;
    RegisterAxisSlavePort(port_name, &axis_mult_portal_slave_interface, DATAW, 0);
}