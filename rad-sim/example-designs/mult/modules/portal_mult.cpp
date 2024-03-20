#include <portal_mult.hpp>

portal_mult::portal_mult(const sc_module_name &name, RADSimDesignContext* radsim_design) //AKB added last arg
    : RADSimModule(name, radsim_design) {

    this->radsim_design = radsim_design;

    // Combinational logic and its sensitivity list
    SC_METHOD(Assign);
    sensitive << rst;

    //maybe add combinational logic if applicable later
    SC_CTHREAD(Tick, clk.pos());
    reset_signal_is(rst, true); // Reset is active high
    this->RegisterModuleInfo(); //can comment out if not connecting to NoC
}


portal_mult::~portal_mult() {}

void portal_mult::Assign() { //combinational logic
    if (rst) {
    portal_axis_slave.tready.write(false);
    axis_mult_portal_slave_interface.tready.write(false);
  } else {
    // Always ready to accept the transaction
    portal_axis_slave.tready.write(true);
    axis_mult_portal_slave_interface.tready.write(true);
  }
}

int counter_mult = 0;
void portal_mult::Tick() { //sequential logic
    portal_out.write(counter_mult);
    wait();
    //Always @ positive edge of clock
    while (true) {
        // Receiving transaction from AXI-S interface
        if (portal_axis_slave.tvalid.read() &&
            portal_axis_slave.tready.read()) {
                //read 
                std::cout << module_name << ": Portal_Mult Module Got Transaction (user (in this case, this device ID) = "
                << portal_axis_slave.tuser.read().to_uint64() << ") (addend = "
                << portal_axis_slave.tdata.read().to_uint64() << ")!"
                << std::endl;
                //write the addend into the mult module and that will flag when received all values and can end simulation
                std::string src_port_name = module_name + ".axis_mult_portal_master_interface";
                std::string dst_port_name = "mult_inst.axis_mult_interface";
                uint64_t dst_addr = radsim_design->GetPortDestinationID(dst_port_name); //AKB changed to ptr deref
                uint64_t src_addr = radsim_design->GetPortDestinationID(src_port_name); //AKB changed to ptr deref
                axis_mult_portal_master_interface.tdest.write(dst_addr);
                axis_mult_portal_master_interface.tid.write(0);
                axis_mult_portal_master_interface.tstrb.write(0);
                axis_mult_portal_master_interface.tkeep.write(0);
                axis_mult_portal_master_interface.tuser.write(src_addr);
                axis_mult_portal_master_interface.tlast.write(portal_axis_slave.tlast.read());
                axis_mult_portal_master_interface.tdata.write(portal_axis_slave.tdata.read());
                axis_mult_portal_master_interface.tvalid.write(true);
                //checking if last transaction and if so, printing current simulation cycle count
                if (portal_axis_slave.tlast.read()) {
                    int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
                    std::cout << "Mult design portal_mult.cpp received last data via inter_rad at cycle " << curr_cycle << std::endl;
                }
        }
        else {
            axis_mult_portal_master_interface.tvalid.write(false);
        }


        //added earlier for testing: writing alternating 0s and 1s to send data directly back to RAD0 with add design
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

    port_name = module_name + ".axis_mult_portal_master_interface";
    //std::cout << port_name << std::endl;
    RegisterAxisMasterPort(port_name, &axis_mult_portal_master_interface, DATAW, 0);
}