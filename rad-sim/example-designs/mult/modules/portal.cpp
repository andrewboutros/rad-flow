#include <portal.hpp>

portal::portal(const sc_module_name &name, RADSimDesignContext* radsim_design) //AKB added last arg
    : RADSimModule(name, radsim_design) {

    //maybe add combinational logic if applicable later
    SC_CTHREAD(Tick, clk.pos());
    //not connecting to NoC
}


portal::~portal() {}

/*void portal::Assign() { //combinational logic
    //maybe add reset signal later
}*/

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
    
}