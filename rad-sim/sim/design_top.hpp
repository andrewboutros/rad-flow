#pragma once

#include <systemc.h>
#include <vector>
#include <portal.hpp>


class RADSimDesignTop : virtual public sc_module {
    public:
        #ifndef SINGLE_RAD
        axis_slave_port design_top_portal_axis_slave;
        axis_master_port design_top_portal_axis_master;
        portal* portal_inst;
        #endif
        RADSimDesignTop(RADSimDesignContext* radsim_design) {
            #ifndef SINGLE_RAD 
            //create portal module
            std::string module_name_str = "portal_inst";
            char module_name[25];
            std::strcpy(module_name, module_name_str.c_str());
            portal_inst = new portal(module_name, radsim_design);

            //connect master to master instead, to expose to top
            portal_inst->portal_axis_master.ConnectToPort(this->design_top_portal_axis_master);
            portal_inst->portal_axis_slave.ConnectToPort(this->design_top_portal_axis_slave); //top drives portal bc top receives slave inputs 
            
            //connect reset signal
            // portal_inst->rst(rst);
            #endif
        }
        ~RADSimDesignTop() {
           #ifndef SINGLE_RAD
           delete portal_inst;
           #endif
        }
        void connectPortalReset(sc_in<bool>* rst) {
            #ifndef SINGLE_RAD
            this->portal_inst->rst(*rst);
            #endif
        }
};