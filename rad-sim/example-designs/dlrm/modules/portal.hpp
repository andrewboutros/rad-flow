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
#include <dlrm_defines.hpp>
#include <sim_utils.hpp> //AKB: added for data_vector template class

struct portal_axis_fields {
            bool tvalid;
            bool tready;
            sc_bv<DATAW> tdata;
            sc_bv<AXIS_STRBW> tstrb;
            sc_bv<AXIS_KEEPW> tkeep;
            bool tlast;
            sc_bv<AXIS_IDW> tid;
            sc_bv<AXIS_DESTW> tdest;
            sc_bv<AXIS_USERW> tuser;
        };

class portal : public RADSimModule {
    private:
        std::queue<portal_axis_fields> portal_axis_fifo;

    public:
        RADSimDesignContext* radsim_design;
        sc_in<bool> rst;
        //sc_in<sc_bv<DATAW>> portal_in;
        //sc_out<sc_bv<DATAW>> portal_out;
        //axis ports for external access to inter_rad
        axis_master_port portal_axis_master;
        axis_slave_port portal_axis_slave;
        //sc_out<bool> portal_recvd; //for testing: flag so add_driver keeps simulation going until data is sent to mult module
        //Interfaces to the NoC
        axis_slave_port axis_portal_slave_interface;
        axis_master_port axis_portal_master_interface;

        portal(const sc_module_name &name, RADSimDesignContext* radsim_design);
        ~portal();

        void Assign(); // Combinational logic process
        void Tick();   // Sequential logic process
        SC_HAS_PROCESS(portal);
        void RegisterModuleInfo(); //even tho did not add AXI Interface, need because is virtual fn in derived class
};

void bv_to_data_vector(
    sc_bv<AXI4_MAX_DATAW> &bitvector, data_vector<int16_t> &datavector,
    unsigned int num_elements);