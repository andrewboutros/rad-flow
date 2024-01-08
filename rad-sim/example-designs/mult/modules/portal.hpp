#pragma once

#include <axis_interface.hpp>
#include <design_context.hpp>
#include <queue>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <client_mult.hpp>
#include <string>
#include <systemc.h>
#include <vector>

class portal : public RADSimModule {
    private:
    public:
    sc_in<bool> portal_in;
    sc_out<bool> portal_out;

    portal(const sc_module_name &name, RADSimDesignContext* radsim_design); //AKB added last arg
    ~portal();

    //void Assign(); // Combinational logic process
    void Tick();   // Sequential logic process
    SC_HAS_PROCESS(portal);
    void RegisterModuleInfo(); //even tho did not add AXI Interface, need because is virtual fn in derived class
};