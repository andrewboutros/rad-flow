#include <server.hpp>

server::server(const sc_module_name &name) : RADSimModule(name) {
	char vserver_name[25];
	std::string vserver_name_str = std::string(name);
	std::strcpy(vserver_name, vserver_name_str.c_str());

	vserver = new Vserver{vserver_name};
	vserver->clk(clk);
	vserver->rst(rst);
	vserver->aximm_server_awvalid(aximm_server.awvalid);
	vserver->aximm_server_awready(aximm_server.awready);
	vserver->aximm_server_awid(aximm_server.awid);
	vserver->aximm_server_awaddr(aximm_server.awaddr);
	vserver->aximm_server_awlen(aximm_server.awlen);
	vserver->aximm_server_awsize(aximm_server.awsize);
	vserver->aximm_server_awburst(aximm_server.awburst);
	vserver->aximm_server_awuser(aximm_server.awuser);
	vserver->aximm_server_wvalid(aximm_server.wvalid);
	vserver->aximm_server_wready(aximm_server.wready);
	vserver->aximm_server_wid(aximm_server.wid);
	vserver->aximm_server_wdata(aximm_server.wdata);
	vserver->aximm_server_wlast(aximm_server.wlast);
	vserver->aximm_server_wuser(aximm_server.wuser);
	vserver->aximm_server_bvalid(aximm_server.bvalid);
	vserver->aximm_server_bready(aximm_server.bready);
	vserver->aximm_server_bid(aximm_server.bid);
	vserver->aximm_server_bresp(aximm_server.bresp);
	vserver->aximm_server_buser(aximm_server.buser);
	vserver->aximm_server_arvalid(aximm_server.arvalid);
	vserver->aximm_server_arready(aximm_server.arready);
	vserver->aximm_server_arid(aximm_server.arid);
	vserver->aximm_server_araddr(aximm_server.araddr);
	vserver->aximm_server_arlen(aximm_server.arlen);
	vserver->aximm_server_arsize(aximm_server.arsize);
	vserver->aximm_server_arburst(aximm_server.arburst);
	vserver->aximm_server_aruser(aximm_server.aruser);
	vserver->aximm_server_rvalid(aximm_server.rvalid);
	vserver->aximm_server_rready(aximm_server.rready);
	vserver->aximm_server_rid(aximm_server.rid);
	vserver->aximm_server_rdata(aximm_server.rdata);
	vserver->aximm_server_rresp(aximm_server.rresp);
	vserver->aximm_server_rlast(aximm_server.rlast);
	vserver->aximm_server_ruser(aximm_server.ruser);

	this->RegisterModuleInfo();
}

server::~server() {
	vserver->final();
	delete vserver;
}

void server::RegisterModuleInfo() {
	std::string port_name;
	_num_noc_axis_slave_ports = 0;
	_num_noc_axis_master_ports = 0;
	_num_noc_aximm_slave_ports = 0;
	_num_noc_aximm_master_ports = 0;

	port_name = module_name + ".aximm_server";
	RegisterAximmSlavePort(port_name, &aximm_server, 512);

}
