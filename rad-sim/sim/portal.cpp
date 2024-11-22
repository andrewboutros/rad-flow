#include <portal.hpp>

portal::portal(const sc_module_name &name, RADSimDesignContext* radsim_design)
    : RADSimModule(name, radsim_design) {

    this->radsim_design = radsim_design;

    //combinational logic
    SC_METHOD(Assign);
    sensitive << rst; // << axis_portal_master_interface.tready; //TODO: add back if inter-rad eventually supports backpressure in this direction
    //sequential logic
    SC_CTHREAD(Tick, clk.pos());
    // This function must be defined & called for any RAD-Sim module to register
    // its info for automatically connecting to the NoC
    reset_signal_is(rst, true); // Reset is active high
    this->RegisterModuleInfo(); //can comment out if not connecting to NoC
}


portal::~portal() {}

void portal::Assign() { //combinational logic
    if (rst) {
        portal_axis_slave.tready.write(false);
        axis_portal_slave_interface.tready.write(false);
    }
    else {
        //Always ready to accept from NoC because we have FIFO buffers in both directions
        //Not exerting back-pressure
        portal_axis_slave.tready.write(true);
        axis_portal_slave_interface.tready.write(true);
    }
}

sc_bv<AXIS_MAX_DATAW> data_to_buffer = 0;
void portal::Tick() { //sequential logic
    portal_axis_master.tvalid.write(false);
    wait();
    //Always @ positive edge of clock
    while (true) {

        //Accepting incoming NoC transaction
        if (axis_portal_slave_interface.tvalid.read() &&
            axis_portal_slave_interface.tready.read()) {
             data_to_buffer = axis_portal_slave_interface.tdata.read();
             portal_axis_fields curr_transaction = {
                axis_portal_slave_interface.tvalid.read(),
                axis_portal_slave_interface.tready.read(),
                axis_portal_slave_interface.tdata.read(),
                axis_portal_slave_interface.tstrb.read(),
                axis_portal_slave_interface.tkeep.read(),
                axis_portal_slave_interface.tlast.read(),
                axis_portal_slave_interface.tid.read(),
                axis_portal_slave_interface.tdest.read(),
                axis_portal_slave_interface.tuser.read()
             };

            portal_axis_fifo_noc_incoming.push(curr_transaction);
        }

        //Sending outgoing inter-rad data
        //warning: must do this before next if-else block so that we pop before reading front. otherwise we get outtdated value on second turn.
        //we see valid as high the clock cycle AFTER we set it as high in the if-else below
        if (portal_axis_master.tvalid.read() && portal_axis_master.tready.read()) { // && test_ready_toggle) { 
            //pop out of fifo
            if (!portal_axis_fifo_noc_incoming.empty()) {
                int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));
                portal_axis_fifo_noc_incoming.pop();

                if (portal_axis_master.tlast.read()) {
                    std::cout << "dlrm design portal.cpp sent last data via inter_rad at cycle " << curr_cycle << std::endl;
                }
            }
            else { //should never reach here because valid should be false if fifo is empty
                std::cout << "reached here but why? portal_axis_fifo_noc_incoming.size(): " << portal_axis_fifo_noc_incoming.size() << std::endl;
            }
        }
        //Prep for sending outgoing inter-rad data
        if ((portal_axis_fifo_noc_incoming.size() > 0) ) { //&& test_ready_toggle) {
            portal_axis_fields curr_transaction = portal_axis_fifo_noc_incoming.front();
            portal_axis_master.tdata.write(curr_transaction.tdata);
            portal_axis_master.tdest.write(curr_transaction.tdest);
            portal_axis_master.tuser.write(curr_transaction.tuser);
            portal_axis_master.tvalid.write(true);
            portal_axis_master.tlast.write(curr_transaction.tlast);
        }
        else {
            portal_axis_master.tdata.write(0);
            portal_axis_master.tvalid.write(false);
        }

        //Accepting incoming inter-rad data and then sending to correct module on RAD over NoC
        if (portal_axis_slave.tvalid.read() &&
            portal_axis_slave.tready.read()) {
             portal_axis_fields curr_transaction = {
                portal_axis_slave.tvalid.read(),
                portal_axis_slave.tready.read(),
                portal_axis_slave.tdata.read(),
                portal_axis_slave.tstrb.read(),
                portal_axis_slave.tkeep.read(),
                portal_axis_slave.tlast.read(),
                portal_axis_slave.tid.read(),
                portal_axis_slave.tdest.read(),
                portal_axis_slave.tuser.read() //tuser field
             };

            portal_axis_fifo_noc_outgoing.push(curr_transaction);
        }

        //Sending outgoing NoC data
        //warning: must do this before next if-else block so that we pop before reading front. otherwise we get outtdated value on second turn.
        //we see valid as high the clock cycle AFTER we set it as high in the if-else below
        if (axis_portal_master_interface.tvalid.read() && axis_portal_master_interface.tready.read()) { // && test_ready_toggle) { 
            //pop out of fifo
            if (!portal_axis_fifo_noc_outgoing.empty()) {
                int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));
                portal_axis_fifo_noc_outgoing.pop();
                if (axis_portal_master_interface.tlast.read()) {
                    std::cout << "dlrm design portal.cpp sent last data via NoC at cycle " << curr_cycle << std::endl;
                }
            }
            else { //should never reach here because valid should be false if fifo is empty
                std::cout << "reached here but why? portal_axis_fifo_noc_outgoing.size(): " << portal_axis_fifo_noc_outgoing.size() << std::endl;
            }
        }
        //Prep for sending outgoing NoC data
        if ((portal_axis_fifo_noc_outgoing.size() > 0) ) { //&& test_ready_toggle) {
            portal_axis_fields curr_transaction = portal_axis_fifo_noc_outgoing.front();
            axis_portal_master_interface.tdata.write(curr_transaction.tdata);
            axis_portal_master_interface.tdest.write(curr_transaction.tdest);
            axis_portal_master_interface.tuser.write(curr_transaction.tuser);
            axis_portal_master_interface.tvalid.write(true);
            axis_portal_master_interface.tlast.write(curr_transaction.tlast);
        }
        else {
            axis_portal_master_interface.tdata.write(0);
            axis_portal_master_interface.tvalid.write(false);
        }


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
    RegisterAxisSlavePort(port_name, &axis_portal_slave_interface, AXIS_MAX_DATAW, 0);
    radsim_design->AssignPortalSlaveName(port_name); //other modules will send to this slave interface

    port_name = module_name + ".axis_portal_master_interface";
    RegisterAxisMasterPort(port_name, &axis_portal_master_interface, AXIS_MAX_DATAW, 0);

}
