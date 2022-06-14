#pragma once

#include <radsim_utils.hpp>
#include <radsim_module.hpp>
#include <radsim_telemetry.hpp>
#include <noc_axis.hpp>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <string>
#include <tuple>

class RADSimDesignContext {
  private:
    std::vector<sc_clock*> _noc_clks;
    std::vector<sc_clock*> _adapter_clks;
    std::vector<sc_clock*> _module_clks;
    std::vector<noc_axis*> _axis_nocs;

    std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, unsigned int>> _port_placement;
    std::unordered_map<std::string, std::pair<unsigned int, unsigned int>> _module_clk_settings;
    std::unordered_map<std::string, radsim_module*> _design_modules;
    std::vector<std::unordered_map<unsigned int, std::vector<std::string>>> _node_id_ports_list;

    std::vector<std::vector<AdapterInfo>> _noc_slave_adapter_info, _noc_master_adapter_info;
    std::vector<std::vector<unsigned int>> _num_noc_slave_ports, _num_noc_master_ports;
    std::vector<std::unordered_map<std::string, axis_slave_port*>> _noc_slave_ports;
    std::vector<std::unordered_map<std::string, axis_master_port*>> _noc_master_ports;
    std::vector<axis_signal> _axis_signals;

  public:
    RADSimDesignContext();
    ~RADSimDesignContext();
    void ParseNoCPlacement(const std::string& placement_filename);
    void ParseClockSettings(const std::string& clks_filename);
    void RegisterModule(std::string module_name, radsim_module* module_ptr);
    void BuildDesignContext(const std::string& placement_filename, const std::string& clks_filename);
    void CreateSystemNoCs(sc_in<bool>& rst);
    void ConnectModulesToNoC();
    void RegisterNoCSlavePort(unsigned int noc_id, std::string& port_name, axis_slave_port* port_ptr);
    void RegisterNoCMasterPort(unsigned int noc_id, std::string& port_name, axis_master_port* port_ptr);
    unsigned int GetNumNoCSlaveAdapters(unsigned int noc_id);
    unsigned int GetNumNoCMasterAdapters(unsigned int noc_id);
    unsigned int GetNumSlaveAdapterPorts(unsigned int noc_id, unsigned int adapter_id);
    unsigned int GetNumMasterAdapterPorts(unsigned int noc_id, unsigned int adapter_id);
    axis_slave_port* GetNoCSlavePort(unsigned int noc_id, std::string& port_name);
    axis_master_port* GetNoCMasterPort(unsigned int noc_id, std::string& port_name);
    std::tuple<unsigned int, unsigned int, unsigned int> GetPortPlacement(std::string& port_name);
    unsigned int GetPortDestinationID(std::string& port_name);
    unsigned int GetPortInterfaceID(std::string& port_name);
    void DumpDesignContext();
};

extern RADSimDesignContext radsim_design;