#pragma once

#include <axis_interface.hpp>
#include <design_context.hpp>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>
#include <radsim_utils.hpp>
#include <fstream>
#include <iostream>

class sender : public RADSimModule {
private:
    std::ofstream outfile;
    int amount_to_send; 
    int wait_cycles;        
public:
    RADSimDesignContext* radsim_design;
    sc_in<bool> rst;    

    // Interface to the NoC
    axis_master_port axis_sender_interface;
    
    sender(const sc_module_name &name, RADSimDesignContext* radsim_design, int amount_to_send = 10, int wait_cycles = 50);
    ~sender();    
    
    void sendData(unsigned int data);
    void Tick();   // Sequential logic process
    SC_HAS_PROCESS(sender);
    void RegisterModuleInfo();
};