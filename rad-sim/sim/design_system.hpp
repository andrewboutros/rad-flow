#pragma once

#include <systemc.h>
#include <design_top.hpp>

class design_system : virtual public sc_module {
    public:
        design_top* design_dut_inst;
};