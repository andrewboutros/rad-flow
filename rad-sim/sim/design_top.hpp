#pragma once

#include <systemc.h>
#include <vector>

//#define DATAW 128

class design_top : virtual public sc_module {
    public:
        //sc_in<sc_bv<DATAW>> portal_in;
        //sc_out<sc_bv<DATAW>> portal_out;
        axis_slave_port design_top_portal_axis_slave; //TODO: add back here, for now just directly putting into child class add_top
        axis_master_port design_top_portal_axis_master;
};