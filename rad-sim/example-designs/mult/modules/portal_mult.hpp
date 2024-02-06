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

class portal_mult : public RADSimModule {
    private:
    public:
    sc_in<sc_bv<DATAW>> portal_in;
    sc_out<sc_bv<DATAW>> portal_out;
    //Interfaces to the NoC
    axis_slave_port axis_mult_portal_slave_interface;

    portal_mult(const sc_module_name &name, RADSimDesignContext* radsim_design); //AKB added last arg
    ~portal_mult();

    //void Assign(); // Combinational logic process
    void Tick();   // Sequential logic process
    SC_HAS_PROCESS(portal_mult);
    void RegisterModuleInfo(); //even tho did not add AXI Interface, need because is virtual fn in derived class
};