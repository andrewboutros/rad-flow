#pragma once

#include <axis_interface.hpp>
#include <design_context.hpp>
#include <queue>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>
#include <radsim_utils.hpp>
#include <tuple>
#include <fstream>
#include <iostream>

class receiver: public RADSimModule {
private:
    std::ifstream refFile;
    bool errorFlag;
public:
    RADSimDesignContext* radsim_design;
    sc_in<bool> rst;    
    // Interface to the NoC
    axis_slave_port axis_receiver_interface;

    receiver(const sc_module_name &name, RADSimDesignContext* radsim_design);
    ~receiver();    
    void Tick();   // Sequential logic process
    SC_HAS_PROCESS(receiver);
    void RegisterModuleInfo();
};