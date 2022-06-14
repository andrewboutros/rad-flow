#pragma once

#include <systemc.h>
#include <string>
#include <unordered_map>
#include <axis_interface.hpp>

class radsim_module : public sc_module {
  public:
    int _num_noc_slave_ports, _num_noc_master_ports;
    std::vector<std::string> _ordered_slave_ports, _ordered_master_ports;
    std::unordered_map<std::string, axis_master_port*> _master_ports;
    std::unordered_map<std::string, axis_slave_port*> _slave_ports;
    std::unordered_map<std::string, unsigned int> _ports_dataw;
    std::unordered_map<std::string, unsigned int> _ports_types;

    sc_in<bool> clk;

    radsim_module(const sc_module_name& name);
    ~radsim_module();
    virtual void RegisterModuleInfo() = 0;
    void RegisterSlavePort(std::string& port_name, axis_slave_port* port_ptr, unsigned int port_dataw, 
      unsigned int port_type);
    void RegisterMasterPort(std::string& port_name, axis_master_port* port_ptr, unsigned int port_dataw, 
      unsigned int port_type);
};