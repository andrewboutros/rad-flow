#pragma once

#include <axis_interface.hpp>
#include <design_context.hpp>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>

class adder : public RADSimModule {
public:
	sc_in<bool> rst;
	sc_out<sc_bv<128>> response;
	sc_out<bool> response_valid;

	axis_slave_port axis_adder_interface;

	adder(const sc_module_name &name);
	~adder();

	SC_HAS_PROCESS(adder);
	void RegisterModuleInfo();
};
