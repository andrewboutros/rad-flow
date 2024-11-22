#pragma once

#include <systemc.h>
#include <design_top.hpp>

class RADSimDesignSystem : virtual public sc_module {
    public:
        RADSimDesignTop* design_dut_inst;
};