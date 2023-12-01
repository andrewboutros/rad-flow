#include <adder.hpp>

adder::adder(const sc_module_name &name) : RADSimModule(name) {
	char vadder_name[25];
	std::string vadder_name_str = std::string(name) + "_vmvm";
	std::strcpy(vadder_name, vadder_name_str.c_str());

	vadder = new Vadder{vadder_name};
	vadder->clk(clk);
	vadder->rst(rst);
	vadder->axis_adder_tvalid(axis_adder.tvalid);
	vadder->axis_adder_tlast(axis_adder.tlast);
	vadder->axis_adder_tdata(axis_adder.tdata);
	vadder->axis_adder_tready(axis_adder.tready);
	vadder->response(response);
	vadder->response_valid(response_valid);

	this->RegisterModuleInfo();
}

adder::~adder() {
	vadder->final();
	delete vadder;
}

void adder::RegisterModuleInfo() {
	std::string port_name;
	_num_noc_axis_slave_ports = 0;
	_num_noc_axis_master_ports = 0;
	_num_noc_aximm_slave_ports = 0;
	_num_noc_aximm_master_ports = 0;

	port_name = module_name + ".axis_adder";
	RegisterAxisSlavePort(port_name, &axis_adder, 512, 0);

}
