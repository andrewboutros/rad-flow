#pragma once

#include <systemc.h>
#include <vector>
#include <portal.hpp>

//#define DATAW 128

class design_top : virtual public sc_module {
    public:
        //sc_in<sc_bv<DATAW>> portal_in;
        //sc_out<sc_bv<DATAW>> portal_out;
        axis_slave_port design_top_portal_axis_slave; //TODO: add back here, for now just directly putting into child class add_top
        axis_master_port design_top_portal_axis_master;
        portal* portal_inst;
        design_top(RADSimDesignContext* radsim_design) { //, sc_in<bool> rst) {
           //create portal module
            std::string module_name_str = "portal_inst";
            char module_name[25];
            std::strcpy(module_name, module_name_str.c_str());
            portal_inst = new portal(module_name, radsim_design);
            // portal_inst->rst(rst); //AKB: commented out to try to fix Info: (I804) /IEEE_Std_1666/deprecated: interface and/or port binding in port constructors is deprecated
            //portal_inst->portal_recvd(this->portal_recvd);

            //connect master to master instead, to expose to top
            portal_inst->portal_axis_master.ConnectToPort(this->design_top_portal_axis_master);
            portal_inst->portal_axis_slave.ConnectToPort(this->design_top_portal_axis_slave); //top drives portal bc top receives slave inputs 
        }
        ~design_top() {
            delete portal_inst;
        }
};