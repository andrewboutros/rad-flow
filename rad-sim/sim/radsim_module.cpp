#include <radsim_module.hpp>
#include <design_context.hpp>

radsim_module::radsim_module(const sc_module_name& name) : sc_module(name) { 
  std::string name_str(static_cast<const char*>(name));
  radsim_design.RegisterModule(name_str, this);
  _num_noc_slave_ports = 0;
  _num_noc_master_ports = 0;
}

radsim_module::~radsim_module(){ }

void radsim_module::RegisterSlavePort(std::string& port_name, axis_slave_port* port_ptr, unsigned int port_dataw, 
  unsigned int port_type) {
  _ordered_slave_ports.push_back(port_name);
  _slave_ports[port_name] = port_ptr;
  _ports_dataw[port_name] = port_dataw;
  _ports_types[port_name] = port_type;
  _num_noc_slave_ports++;
}

void radsim_module::RegisterMasterPort(std::string& port_name, axis_master_port* port_ptr, unsigned int port_dataw, 
  unsigned int port_type) {
  _ordered_master_ports.push_back(port_name);
  _master_ports[port_name] = port_ptr;
  _ports_dataw[port_name] = port_dataw;
  _ports_types[port_name] = port_type;
  _num_noc_master_ports++;
}