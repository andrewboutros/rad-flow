#include <design_context.hpp>
#include <radsim_module.hpp>

RADSimModule::RADSimModule(const sc_module_name &name) : sc_module(name) {
  module_name = name;
  std::string name_str(static_cast<const char *>(name));
  radsim_design.RegisterModule(name_str, this);
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;
}

RADSimModule::~RADSimModule() {}

void RADSimModule::RegisterAxisSlavePort(std::string &port_name,
                                          axis_slave_port *port_ptr,
                                          unsigned int port_dataw,
                                          unsigned int port_type) {
  _ordered_axis_slave_ports.push_back(port_name);
  _axis_slave_ports[port_name] = port_ptr;
  _ports_dataw[port_name] = port_dataw;
  _ports_types[port_name] = port_type;
  _ports_is_aximm[port_name] = false;
  _num_noc_axis_slave_ports++;
}

void RADSimModule::RegisterAxisMasterPort(std::string &port_name,
                                           axis_master_port *port_ptr,
                                           unsigned int port_dataw,
                                           unsigned int port_type) {
  _ordered_axis_master_ports.push_back(port_name);
  _axis_master_ports[port_name] = port_ptr;
  _ports_dataw[port_name] = port_dataw;
  _ports_types[port_name] = port_type;
  _ports_is_aximm[port_name] = false;
  _num_noc_axis_master_ports++;
}

void RADSimModule::RegisterAximmSlavePort(std::string &port_name,
                                           aximm_slave_port *port_ptr,
                                           unsigned int port_dataw) {
  _ordered_aximm_slave_ports.push_back(port_name);
  _aximm_slave_ports[port_name] = port_ptr;
  _ports_dataw[port_name] = port_dataw;
  _ports_types[port_name] = 0;
  _ports_is_aximm[port_name] = true;
  _num_noc_aximm_slave_ports++;
}

void RADSimModule::RegisterAximmMasterPort(std::string &port_name,
                                            aximm_master_port *port_ptr,
                                            unsigned int port_dataw) {
  _ordered_aximm_master_ports.push_back(port_name);
  _aximm_master_ports[port_name] = port_ptr;
  _ports_dataw[port_name] = port_dataw;
  _ports_types[port_name] = 0;
  _ports_is_aximm[port_name] = true;
  _num_noc_aximm_master_ports++;
}