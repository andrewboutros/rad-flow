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

struct portal_axis_fields {
            bool tvalid;
            bool tready;
            sc_bv<AXIS_MAX_DATAW> tdata;
            sc_bv<AXIS_STRBW> tstrb;
            sc_bv<AXIS_KEEPW> tkeep;
            bool tlast;
            sc_bv<AXIS_IDW> tid;
            sc_bv<AXIS_DESTW> tdest;
            sc_bv<AXIS_USERW> tuser;
        };


//The portal class is a module that connects from a device's NoC to the outer inter-RAD network.
class portal : public RADSimModule {
    private:
        std::queue<portal_axis_fields> portal_axis_fifo_noc_incoming;
        std::queue<portal_axis_fields> portal_axis_fifo_noc_outgoing;

    public:
        RADSimDesignContext* radsim_design;
        sc_in<bool> rst { "rst" };
        //axis ports for external access to inter_rad
        axis_master_port portal_axis_master;
        axis_slave_port portal_axis_slave;
        //Interfaces to the NoC
        axis_slave_port axis_portal_slave_interface;
        axis_master_port axis_portal_master_interface;

        portal(const sc_module_name &name, RADSimDesignContext* radsim_design);
        ~portal();

        void Assign(); // Combinational logic process
        void Tick();   // Sequential logic process
        SC_HAS_PROCESS(portal);
        void RegisterModuleInfo();
};