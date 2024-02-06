#pragma once

#include <systemc.h>
#include <vector>

#define DATAW 128

class design_top : virtual public sc_module {
    public:
        sc_in<sc_bv<DATAW>> portal_in;
        sc_out<sc_bv<DATAW>> portal_out;
        axis_slave_port* top_axis_portal_interface;
};