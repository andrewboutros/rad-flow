#pragma once

#include <axis_interface.hpp>
#include <design_context.hpp>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>

#include <Vclient.h>
class client : public RADSimModule {
private:
	Vclient* vclient;

public:
	sc_in<bool> rst;
	sc_in<sc_bv<128>> client_tdata;
	sc_in<bool> client_tlast;
	sc_in<bool> client_valid;
	sc_out<bool> client_ready;

	axis_master_port axis_client_interface;

	client(const sc_module_name &name);
	~client();

	SC_HAS_PROCESS(client);
	void RegisterModuleInfo();
};
