#include <Vadder.h>
#include <adder.hpp>

adder::adder(const sc_module_name &name) : RADSimModule(name) {
	Vadder* vadder = new Vadder{"vadder"};
	vadder->clk(clk);
	vadder->rst(rst);
	vadder->axis_adder_interface_tvalid(axis_adder_interface.tvalid);
	vadder->axis_adder_interface_tlast(axis_adder_interface.tlast);
	vadder->axis_adder_interface_tdata(axis_adder_interface.tdata);
	vadder->axis_adder_interface_tready(axis_adder_interface.tready);
	vadder->response(response);
	vadder->response_valid(response_valid);

	this->RegisterModuleInfo();
}

adder::~adder() {}

void adder::RegisterModuleInfo() {
	std::string port_name;
	_num_noc_axis_slave_ports = 0;
	_num_noc_axis_master_ports = 0;
	_num_noc_aximm_slave_ports = 0;
	_num_noc_aximm_master_ports = 0;
	port_name = module_name + ".axis_adder_interface";
	RegisterAxisSlavePort(port_name, &axis_adder_interface, 128, 0);
}
