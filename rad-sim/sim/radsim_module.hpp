#pragma once

#include <aximm_interface.hpp>
#include <axis_interface.hpp>
#include <string>
#include <systemc.h>
#include <unordered_map>

class RADSimDesignContext;

class RADSimModule : public sc_module {
public:
  std::string module_name;
  int _num_noc_axis_slave_ports, _num_noc_axis_master_ports;
  int _num_noc_aximm_slave_ports, _num_noc_aximm_master_ports;
  std::vector<std::string> _ordered_axis_slave_ports;
  std::vector<std::string> _ordered_axis_master_ports;
  std::vector<std::string> _ordered_aximm_slave_ports;
  std::vector<std::string> _ordered_aximm_master_ports;
  std::unordered_map<std::string, axis_master_port *> _axis_master_ports;
  std::unordered_map<std::string, axis_slave_port *> _axis_slave_ports;
  std::unordered_map<std::string, aximm_master_port *> _aximm_master_ports;
  std::unordered_map<std::string, aximm_slave_port *> _aximm_slave_ports;
  std::unordered_map<std::string, unsigned int> _ports_dataw;
  std::unordered_map<std::string, unsigned int> _ports_types;
  std::unordered_map<std::string, bool> _ports_is_aximm;

  sc_in<bool> clk;

  RADSimModule(const sc_module_name &name, RADSimDesignContext* radsim_design);
  ~RADSimModule();
  virtual void RegisterModuleInfo() = 0;
  void RegisterAxisSlavePort(std::string &port_name, axis_slave_port *port_ptr,
                             unsigned int port_dataw, unsigned int port_type);
  void RegisterAxisMasterPort(std::string &port_name,
                              axis_master_port *port_ptr,
                              unsigned int port_dataw, unsigned int port_type);
  void RegisterAximmSlavePort(std::string &port_name,
                              aximm_slave_port *port_ptr,
                              unsigned int port_dataw);
  void RegisterAximmMasterPort(std::string &port_name,
                               aximm_master_port *port_ptr,
                               unsigned int port_dataw);
};