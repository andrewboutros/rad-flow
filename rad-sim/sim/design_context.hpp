#pragma once

#include <iostream>
#include <radsim_module.hpp>
#include <radsim_noc.hpp>
#include <radsim_telemetry.hpp>
#include <radsim_utils.hpp>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

class RADSimDesignContext {
private:
  int _design_result = 0;

  std::vector<sc_clock *> _noc_clks;
  std::vector<sc_clock *> _adapter_clks;
  std::vector<sc_clock *> _module_clks;
  std::vector<radsim_noc *> _nocs;

  std::unordered_map<std::string,
                     std::tuple<unsigned int, unsigned int, unsigned int>>
      _port_placement;
  std::unordered_map<std::string, std::pair<unsigned int, unsigned int>>
      _module_clk_settings;
  std::unordered_map<std::string, RADSimModule *> _design_modules;
  std::vector<std::unordered_map<unsigned int, bool>> _node_id_is_aximm;
  std::vector<std::unordered_map<unsigned int, std::vector<std::string>>>
      _node_id_ports_list;
  std::vector<std::vector<std::set<std::string>>> _node_module_names;
  std::unordered_map<std::string, uint64_t> _aximm_port_base_addresses;

  std::vector<std::vector<AdapterInfo>> _noc_axis_slave_adapter_info,
      _noc_axis_master_adapter_info;
  std::vector<std::vector<AdapterInfo>> _noc_aximm_slave_adapter_info,
      _noc_aximm_master_adapter_info;

  // AXI-S Adapters & Ports
  std::vector<std::vector<unsigned int>> _num_noc_axis_slave_ports,
      _num_noc_axis_master_ports;
  std::vector<std::unordered_map<std::string, axis_slave_port *>>
      _noc_axis_slave_ports;
  std::vector<std::unordered_map<std::string, axis_master_port *>>
      _noc_axis_master_ports;
  std::vector<axis_signal> _axis_signals;

  // AXI-MM Adapters & Ports
  std::vector<std::vector<unsigned int>> _num_noc_aximm_slave_ports,
      _num_noc_aximm_master_ports;
  std::vector<std::unordered_map<std::string, aximm_slave_port *>>
      _noc_aximm_slave_ports;
  std::vector<std::unordered_map<std::string, aximm_master_port *>>
      _noc_aximm_master_ports;
  std::vector<aximm_signal> _aximm_signals;

public:
  RADSimDesignContext();
  ~RADSimDesignContext();
  void ParseNoCPlacement(const std::string &placement_filename);
  void ParseClockSettings(const std::string &clks_filename);
  void RegisterModule(std::string module_name, RADSimModule *module_ptr);
  void BuildDesignContext(const std::string &placement_filename,
                          const std::string &clks_filename);
  void CreateSystemNoCs(sc_in<bool> &rst);
  void ConnectModulesToNoC();

  void RegisterNoCSlavePort(unsigned int noc_id, std::string &port_name,
                            axis_slave_port *port_ptr);
  void RegisterNoCMasterPort(unsigned int noc_id, std::string &port_name,
                             axis_master_port *port_ptr);
  void RegisterNoCSlavePort(unsigned int noc_id, std::string &port_name,
                            aximm_slave_port *port_ptr);
  void RegisterNoCMasterPort(unsigned int noc_id, std::string &port_name,
                             aximm_master_port *port_ptr);

  unsigned int GetNumNoCSlaveAdapters(unsigned int noc_id, bool is_aximm);
  unsigned int GetNumNoCMasterAdapters(unsigned int noc_id, bool is_aximm);

  unsigned int GetNumAxisSlaveAdapterPorts(unsigned int noc_id,
                                           unsigned int adapter_id);
  unsigned int GetNumAxisMasterAdapterPorts(unsigned int noc_id,
                                            unsigned int adapter_id);

  axis_slave_port *GetNoCAxisSlavePort(unsigned int noc_id,
                                       std::string &port_name);
  axis_master_port *GetNoCAxisMasterPort(unsigned int noc_id,
                                         std::string &port_name);
  aximm_slave_port *GetNoCAximmSlavePort(unsigned int noc_id,
                                         std::string &port_name);
  aximm_master_port *GetNoCAximmMasterPort(unsigned int noc_id,
                                           std::string &port_name);

  std::tuple<unsigned int, unsigned int, unsigned int>
  GetPortPlacement(std::string &port_name);
  unsigned int GetPortDestinationID(std::string &port_name);
  unsigned int GetPortInterfaceID(std::string &port_name);
  void DumpDesignContext();
  std::vector<std::vector<std::set<std::string>>> &GetNodeModuleNames();
  uint64_t GetPortBaseAddress(std::string &port_name);

  int GetDesignResult();
  void ReportDesignFailure();
};

extern RADSimDesignContext radsim_design;