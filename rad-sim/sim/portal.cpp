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
        portal_axis_slave.tready.write(true); //axis_portal_master_interface.tready.read()) //TODO: replace if support backpressure onto inter-rad
        axis_portal_slave_interface.tready.write(true);
    }
}

//used for internal testing
//must uncomment radsim_utils define to use
// void bv_to_data_vector(
//     sc_bv<AXI4_MAX_DATAW> &bitvector, data_vector<int16_t> &datavector,
//     unsigned int num_elements) {

//   unsigned int start_idx, end_idx;
//   unsigned int _bitwidth = 16; //AKB: extra added
//   for (unsigned int e = 0; e < num_elements; e++) {
//     start_idx = e * _bitwidth;
//     end_idx = (e + 1) * _bitwidth;
//     datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
//   }
// }

int counter = 0;
sc_bv<AXIS_MAX_DATAW> data_to_buffer = 0;
sc_bv<AXIS_USERW> dest_device = 1; //for testing, fixed at 1 to send to RAD1 which has mult design; //#define AXIS_USERW     66
//bool got_data = false;
void portal::Tick() { //sequential logic
    //portal_recvd.write(0);
    portal_axis_master.tvalid.write(false);
    //bool test_ready_toggle = false;
    wait();
    //Always @ positive edge of clock
    while (true) {

        int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));

        //Accepting incoming NoC transaction
        if (axis_portal_slave_interface.tvalid.read() &&
            axis_portal_slave_interface.tready.read()) {
            //std::cout << "Also got here" << std:: endl;
            // std::cout << "DLRM design raising valid data to send over portal module on cycle " << curr_cycle << " , will see valid high next clk cycle " << module_name << ": Got Transaction (user = "
            //             << axis_portal_slave_interface.tuser.read().to_uint64() << ") (value = "
            //             << axis_portal_slave_interface.tdata.read().to_uint64() << ")! Destination field is "
            //             << axis_portal_slave_interface.tdest.read().to_uint64()
            //             << std::endl;
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
                axis_portal_slave_interface.tuser.read() //tuser field
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
                /* START FOR DEBUG */
		        //sc_bv<AXIS_MAX_DATAW> tx_tdata_bv = portal_axis_fifo_noc_incoming.front().tdata;
                //data_vector<int16_t> tx_tdata(32);
                //bv_to_data_vector(tx_tdata_bv, tx_tdata, 32);
                //std::cout << "portal @ cycle " << curr_cycle << ": sending over inter-RAD" << tx_tdata << " from RAD " << radsim_design->rad_id << std::endl;
                /* END FOR DEBUG */

                portal_axis_fifo_noc_incoming.pop();
                /* START FOR DEBUG */
                //std::cout << "portal.cpp in dlrm design sent " << portal_axis_master.tdata.read().to_uint64() << " to dest_device " << dest_device.to_uint64() << " on cycle " << curr_cycle << std::endl;
                /* END FOR DEBUG */
                //portal_recvd.write(1);

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
            //counter++;
            portal_axis_master.tdata.write(0);
            //portal_axis_master.tuser.write(dest_device);
            portal_axis_master.tvalid.write(false);
        }

        //Accepting incoming inter-rad data and then sending to correct module on RAD over NoC
        // if (portal_axis_slave.tvalid.read() && //tvalid is written by inter-rad module
        //     portal_axis_slave.tready.read()) { //tready is written by this portal module
        //         //get current cycle
        //         int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
        //         //read 
        //         sc_bv<DATAW> rx_tdata_bv = portal_axis_slave.tdata.read();
        //         data_vector<int16_t> rx_tdata(32);
        //         bv_to_data_vector(rx_tdata_bv, rx_tdata, 32);
        //         std::cout << module_name << ": Portal Module Got Transaction on cycle " << curr_cycle << " (RAD ID) = "
        //         << radsim_design->rad_id  //<< portal_axis_slave.tuser.read().to_uint64() 
        //         << ") (val = " //<< portal_axis_slave.tdata.read().to_uint64() << ")!"
        //         << rx_tdata << ") with tdest field of "
        //         << portal_axis_slave.tdest.read() << "!"
        //         << std::endl;
        //         //write the addend into the mult module and that will flag when received all values and can end simulation
        //         std::string src_port_name = module_name + ".axis_portal_master_interface";
        //         uint64_t src_addr = radsim_design->GetPortDestinationID(src_port_name); //AKB changed to ptr deref
        //         //sc_bv<AXIS_DESTW> concat_dest = portal_axis_slave.tdest.read();
        //         //DEST_RAD(concat_dest) = radsim_design->rad_id;
        //         //DEST_LOCAL_NODE(concat_dest) = //dst_addr;
        //         //std::cout << "portal_axis_slave.tdest.read() is: " << portal_axis_slave.tdest.read() << std::endl;
        //         axis_portal_master_interface.tdest.write(portal_axis_slave.tdest.read()); //concat_dest); //dst_addr);
        //         axis_portal_master_interface.tid.write(0);
        //         axis_portal_master_interface.tstrb.write(0);
        //         axis_portal_master_interface.tkeep.write(0);
        //         axis_portal_master_interface.tuser.write(portal_axis_slave.tuser.read());
        //         //std::cout << "portal_axis_slave.tuser.read()" << portal_axis_slave.tuser.read().range(15, 13).to_uint() << std::endl;
        //         axis_portal_master_interface.tlast.write(portal_axis_slave.tlast.read());
        //         axis_portal_master_interface.tdata.write(portal_axis_slave.tdata.read());
        //         axis_portal_master_interface.tvalid.write(true);
        //         //checking if last transaction and if so, printing current simulation cycle count
        //         if (portal_axis_slave.tlast.read()) {
        //             std::cout << "portal.cpp received last data via inter_rad at cycle " << curr_cycle << std::endl;
        //         }
        // }
        // else {
        //     axis_portal_master_interface.tvalid.write(false);
        //     std::cout << "portal_axis_slave.tvalid.read(): " << portal_axis_slave.tvalid.read()
        //     << " portal_axis_slave.tready.read() " << portal_axis_slave.tready.read() 
        //     << " axis_portal_master_interface.tready.read() " << axis_portal_master_interface.tready.read() << std::endl;
        // }
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
                //used for testing
		//sc_bv<AXIS_MAX_DATAW> tx_tdata_bv = portal_axis_fifo_noc_outgoing.front().tdata;
                //data_vector<int16_t> tx_tdata(32);
                //bv_to_data_vector(tx_tdata_bv, tx_tdata, 32);
                //std::cout << "portal @ cycle " << curr_cycle << ": sending over NoC " << tx_tdata << " from RAD " << radsim_design->rad_id << std::endl;
                
                portal_axis_fifo_noc_outgoing.pop();
                //std::cout << "portal.cpp in dlrm design sent " << portal_axis_master.tdata.read().to_uint64() << " to dest_device " << dest_device.to_uint64() << " on cycle " << curr_cycle << std::endl;
                //portal_recvd.write(1);
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
            //counter++;
            axis_portal_master_interface.tdata.write(0);
            //portal_axis_master.tuser.write(dest_device);
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
    //radsim_design->portal_id = radsim_design->GetPortDestinationID(port_name); //store slave port info
    //radsim_design->AssignPortalSlaveID(radsim_design->GetPortDestinationID(port_name));
    radsim_design->AssignPortalSlaveName(port_name); //bc other modules will send to this slave interface

    port_name = module_name + ".axis_portal_master_interface";
    RegisterAxisMasterPort(port_name, &axis_portal_master_interface, AXIS_MAX_DATAW, 0);

}
