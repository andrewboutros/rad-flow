#include <client.hpp>

client::client(const sc_module_name &name) : RADSimModule(name) {
	char vclient_name[25];
	std::string vclient_name_str = std::string(name);
	std::strcpy(vclient_name, vclient_name_str.c_str());

	vclient = new Vclient{vclient_name};
	vclient->clk(clk);
	vclient->rst(rst);
	vclient->client_tdata(client_tdata);
	vclient->client_tlast(client_tlast);
	vclient->client_valid(client_valid);
	vclient->axis_client_tready(axis_client.tready);
	vclient->client_ready(client_ready);
	vclient->axis_client_tvalid(axis_client.tvalid);
	vclient->axis_client_tlast(axis_client.tlast);
	vclient->axis_client_tdest(axis_client.tdest);
	vclient->axis_client_tid(axis_client.tid);
	vclient->axis_client_tstrb(axis_client.tstrb);
	vclient->axis_client_tkeep(axis_client.tkeep);
	vclient->axis_client_tuser(axis_client.tuser);
	vclient->axis_client_tdata(axis_client.tdata);

	this->RegisterModuleInfo();
}

client::~client() {
	vclient->final();
	delete vclient;
}

void client::RegisterModuleInfo() {
	std::string port_name;
	_num_noc_axis_slave_ports = 0;
	_num_noc_axis_master_ports = 0;
	_num_noc_aximm_slave_ports = 0;
	_num_noc_aximm_master_ports = 0;

	port_name = module_name + ".axis_client";
	RegisterAxisMasterPort(port_name, &axis_client, 512, 0);

}
