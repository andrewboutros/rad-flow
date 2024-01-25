#pragma once

#include <systemc.h>

class design_top : virtual public sc_module {
    public:
        sc_in<bool> portal_in;
        sc_out<bool> portal_out;
};