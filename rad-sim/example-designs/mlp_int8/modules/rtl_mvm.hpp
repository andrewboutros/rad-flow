#pragma once

#include <axis_interface.hpp>
#include <design_context.hpp>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>

#include <Vrtl_mvm.h>
class rtl_mvm : public RADSimModule {
private:
	Vrtl_mvm* vrtl_mvm;

public:
	sc_in<bool> rst;

	axis_slave_port axis_rx;
	axis_master_port axis_tx;

	rtl_mvm(const sc_module_name &name);
	~rtl_mvm();

	SC_HAS_PROCESS(rtl_mvm);
	void RegisterModuleInfo();
};
