#pragma once

#include <systemc.h>
#include <vector>
#include <portal.hpp>


class RADSimDesignTop : virtual public sc_module {
    public:
        axis_slave_port design_top_portal_axis_slave;
        axis_master_port design_top_portal_axis_master;
        portal* portal_inst;
        RADSimDesignTop(RADSimDesignContext* radsim_design) { 
           //create portal module
            std::string module_name_str = "portal_inst";
            char module_name[25];
            std::strcpy(module_name, module_name_str.c_str());
            portal_inst = new portal(module_name, radsim_design);

            //connect master to master instead, to expose to top
            portal_inst->portal_axis_master.ConnectToPort(this->design_top_portal_axis_master);
            portal_inst->portal_axis_slave.ConnectToPort(this->design_top_portal_axis_slave); //top drives portal bc top receives slave inputs 
        }
        ~RADSimDesignTop() {
            delete portal_inst;
        }
};