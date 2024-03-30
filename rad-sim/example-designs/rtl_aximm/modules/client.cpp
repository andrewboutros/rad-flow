#include <client.hpp>

client::client(const sc_module_name &name) : RADSimModule(name) {
	char vclient_name[25];
	std::string vclient_name_str = std::string(name);
	std::strcpy(vclient_name, vclient_name_str.c_str());

	vclient = new Vclient{vclient_name};
	vclient->clk(clk);
	vclient->rst(rst);
	vclient->start_sig(start_sig);
	vclient->start_rdy(start_rdy);
	vclient->output_valid(output_valid);
	vclient->output_result(output_result);
	vclient->output_rdy(output_rdy);
	vclient->aximm_client_awvalid(aximm_client.awvalid);
	vclient->aximm_client_awready(aximm_client.awready);
	vclient->aximm_client_awid(aximm_client.awid);
	vclient->aximm_client_awaddr(aximm_client.awaddr);
	vclient->aximm_client_awlen(aximm_client.awlen);
	vclient->aximm_client_awsize(aximm_client.awsize);
	vclient->aximm_client_awburst(aximm_client.awburst);
	vclient->aximm_client_awuser(aximm_client.awuser);
	vclient->aximm_client_wvalid(aximm_client.wvalid);
	vclient->aximm_client_wready(aximm_client.wready);
	vclient->aximm_client_wid(aximm_client.wid);
	vclient->aximm_client_wdata(aximm_client.wdata);
	vclient->aximm_client_wlast(aximm_client.wlast);
	vclient->aximm_client_wuser(aximm_client.wuser);
	vclient->aximm_client_bvalid(aximm_client.bvalid);
	vclient->aximm_client_bready(aximm_client.bready);
	vclient->aximm_client_bid(aximm_client.bid);
	vclient->aximm_client_bresp(aximm_client.bresp);
	vclient->aximm_client_buser(aximm_client.buser);
	vclient->aximm_client_arvalid(aximm_client.arvalid);
	vclient->aximm_client_arready(aximm_client.arready);
	vclient->aximm_client_arid(aximm_client.arid);
	vclient->aximm_client_araddr(aximm_client.araddr);
	vclient->aximm_client_arlen(aximm_client.arlen);
	vclient->aximm_client_arsize(aximm_client.arsize);
	vclient->aximm_client_arburst(aximm_client.arburst);
	vclient->aximm_client_aruser(aximm_client.aruser);
	vclient->aximm_client_rvalid(aximm_client.rvalid);
	vclient->aximm_client_rready(aximm_client.rready);
	vclient->aximm_client_rid(aximm_client.rid);
	vclient->aximm_client_rdata(aximm_client.rdata);
	vclient->aximm_client_rresp(aximm_client.rresp);
	vclient->aximm_client_rlast(aximm_client.rlast);
	vclient->aximm_client_ruser(aximm_client.ruser);

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

	port_name = module_name + ".aximm_client";
	RegisterAximmMasterPort(port_name, &aximm_client, 512);

}
