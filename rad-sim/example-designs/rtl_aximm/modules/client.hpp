#pragma once

#include <aximm_interface.hpp>
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
	sc_in<bool> start_sig;
	sc_out<bool> start_rdy;
	sc_out<bool> output_valid;
	sc_out<bool> output_result;
	sc_in<bool> output_rdy;

	aximm_master_port aximm_client;

	client(const sc_module_name &name);
	~client();

	SC_HAS_PROCESS(client);
	void RegisterModuleInfo();
};
