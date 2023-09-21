#include <client_wrapper.hpp>
#include <Vclient.h>

client_wrapper::client_wrapper(const sc_module_name &name) : radsim_module(name) {
  Vclient* vclient = new Vclient{"vclient"};

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

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

client_wrapper::~client_wrapper() {}

void client_wrapper::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;

  port_name = module_name + ".axis_client_interface";
  RegisterAxisMasterPort(port_name, &axis_client_interface, DATAW, 0);
}