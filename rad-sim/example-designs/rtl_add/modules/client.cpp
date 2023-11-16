#include <client.hpp>

client::client(const sc_module_name &name) : RADSimModule(name) {
	char vclient_name[25];
	std::string vclient_name_str = std::string(name) + "_vmvm";
	std::strcpy(vclient_name, vclient_name_str.c_str());

	vclient = new Vclient{vclient_name};
	vclient->clk(clk);
	vclient->rst(rst);
	vclient->client_tdata(client_tdata);
	vclient->client_tlast(client_tlast);
	vclient->client_valid(client_valid);
	vclient->axis_client_interface_tready(axis_client_interface.tready);
	vclient->client_ready(client_ready);
	vclient->axis_client_interface_tvalid(axis_client_interface.tvalid);
	vclient->axis_client_interface_tlast(axis_client_interface.tlast);
	vclient->axis_client_interface_tdest(axis_client_interface.tdest);
	vclient->axis_client_interface_tid(axis_client_interface.tid);
	vclient->axis_client_interface_tstrb(axis_client_interface.tstrb);
	vclient->axis_client_interface_tkeep(axis_client_interface.tkeep);
	vclient->axis_client_interface_tuser(axis_client_interface.tuser);
	vclient->axis_client_interface_tdata(axis_client_interface.tdata);

	this->RegisterModuleInfo();
}

client::~client() {
	delete vclient;
}

void client::RegisterModuleInfo() {
	std::string port_name;
	_num_noc_axis_slave_ports = 0;
	_num_noc_axis_master_ports = 0;
	_num_noc_aximm_slave_ports = 0;
	_num_noc_aximm_master_ports = 0;

	port_name = module_name + ".axis_client_interface";
	RegisterAxisMasterPort(port_name, &axis_client_interface, 512, 0);

}
