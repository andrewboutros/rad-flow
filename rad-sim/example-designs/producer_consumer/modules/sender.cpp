#include <sender.hpp>

sender::sender(const sc_module_name &name, RADSimDesignContext* radsim_design, int amount_to_send, int wait_cycles)
      : RADSimModule(name, radsim_design) {
    this->radsim_design = radsim_design;
    this->amount_to_send = amount_to_send;
    this->wait_cycles = wait_cycles;

    SC_CTHREAD(Tick, clk.pos());
    reset_signal_is(rst, true); // Reset is active high
    
    RegisterModuleInfo();
    outfile.open("sender_output.txt");
}

sender::~sender() {
    // Destructor does not need to do anything for this module
    if (outfile.is_open()) {
        outfile.close();
    }
}

void sender::sendData(unsigned int data) {    
    std::string src_port_name = module_name + ".axis_sender_interface";
    std::string dst_port_name = "receiver_inst.axis_receiver_interface";
    uint64_t dst_addr = radsim_design->GetPortDestinationID(dst_port_name);
    uint64_t src_addr = radsim_design->GetPortDestinationID(src_port_name);
    sc_bv<AXIS_DESTW> dest_id_concat;
    DEST_REMOTE_NODE(dest_id_concat) = 0; // staying on same RAD
    DEST_LOCAL_NODE(dest_id_concat) = dst_addr;
    DEST_RAD(dest_id_concat) = radsim_design->rad_id;       

    axis_sender_interface.tdest.write(dest_id_concat);
    axis_sender_interface.tid.write(0);
    axis_sender_interface.tstrb.write(0);
    axis_sender_interface.tkeep.write(0);
    axis_sender_interface.tuser.write(src_addr);
    
    if (amount_to_send > 1) {
        axis_sender_interface.tlast.write(0); // Not the last data
    } else {
        axis_sender_interface.tlast.write(1); // Last data
    }
    
    axis_sender_interface.tdata.write(data);
}

void sender::Tick() {
    if(rst) {
        axis_sender_interface.tvalid.write(false);  
    }
    wait();

    int data_left = amount_to_send;
    cout << "Sender started with amount to send: " << amount_to_send << endl;
    unsigned int placeholderData = 1;
    bool firstTransaction = true;
    axis_sender_interface.tvalid.write(true);

    while (data_left > 0) {    
        
        double sim_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));
        if(firstTransaction) {
            sendData(placeholderData);
            std::cout << "@cycle " << sim_cycle << " Sending first data: " << placeholderData << std::endl;
            firstTransaction = false;
            outfile << placeholderData << std::endl;
            placeholderData += 1;
            data_left--;                     
        }
        else if(axis_sender_interface.tready.read()) {            
            sendData(placeholderData);
            std::cout << "@cycle " << sim_cycle << " Sending data: " << placeholderData << std::endl;
            outfile << placeholderData << std::endl;
            placeholderData += 1;
            data_left--;
        }
        wait();      
        
    }

    //wait for final ready signal before turning off the valid signal
    while(!axis_sender_interface.tready.read()) {
        wait();
    }
    axis_sender_interface.tvalid.write(false); // No more data to send
    int wait_start_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));
    int curr_cycle = wait_start_cycle;

    while(curr_cycle < wait_start_cycle + wait_cycles) {
        curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));
        wait();
    }
    this->radsim_design->set_rad_done();
}


void sender::RegisterModuleInfo() {
    std::string port_name = module_name + ".axis_sender_interface";
    RegisterAxisMasterPort(port_name, &axis_sender_interface, 128, 0);
}



