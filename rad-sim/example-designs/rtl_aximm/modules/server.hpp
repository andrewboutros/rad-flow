#pragma once

#include <aximm_interface.hpp>
#include <design_context.hpp>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>

#include <Vserver.h>
class server : public RADSimModule {
private:
	Vserver* vserver;

public:
	sc_in<bool> rst;

	aximm_slave_port aximm_server;

	server(const sc_module_name &name);
	~server();

	SC_HAS_PROCESS(server);
	void RegisterModuleInfo();
};
