#include <design_context.hpp>

RADSimDesignContext::RADSimDesignContext() {
  std::string radsim_knobs_filename = "/sim/radsim_knobs";
  std::string radsim_knobs_filepath = RADSIM_ROOT_DIR + radsim_knobs_filename;
  ParseRADSimKnobs(radsim_knobs_filepath);

  // Create NoC clocks
  std::string clk_name;
  std::vector<double> noc_period =
      radsim_config.GetDoubleVectorKnob("noc_clk_period");
  _noc_clks.resize(noc_period.size());
  for (unsigned int clk_id = 0; clk_id < _noc_clks.size(); clk_id++) {
    clk_name = "noc_clk" + std::to_string(clk_id);
    _noc_clks[clk_id] =
        new sc_clock(clk_name.c_str(), noc_period[clk_id], SC_NS);
  }

  // Create adapter clocks
  std::vector<double> adapter_period =
      radsim_config.GetDoubleVectorKnob("noc_adapters_clk_period");
  _adapter_clks.resize(adapter_period.size());
  for (unsigned int clk_id = 0; clk_id < _adapter_clks.size(); clk_id++) {
    clk_name = "adapter_clk" + std::to_string(clk_id);
    _adapter_clks[clk_id] =
        new sc_clock(clk_name.c_str(), adapter_period[clk_id], SC_NS);
  }

  // Create module clocks
  std::vector<double> module_period =
      radsim_config.GetDoubleVectorKnob("design_clk_periods");
  _module_clks.resize(module_period.size());
  for (unsigned int clk_id = 0; clk_id < _module_clks.size(); clk_id++) {
    clk_name = "module_clk" + std::to_string(clk_id);
    _module_clks[clk_id] =
        new sc_clock(clk_name.c_str(), module_period[clk_id], SC_NS);
  }

  int num_nocs = radsim_config.GetIntKnob("noc_num_nocs");
  _node_module_names.resize(num_nocs);
  for (int noc_id = 0; noc_id < num_nocs; noc_id++) {
    int num_nodes = radsim_config.GetIntVectorKnob("noc_num_nodes", noc_id);
    _node_module_names[noc_id].resize(num_nodes);
  }
}

RADSimDesignContext::~RADSimDesignContext() {}

bool IsSlavePort(std::string &port_name, RADSimModule *module_ptr) {
  bool is_axis_slave = (module_ptr->_axis_slave_ports.find(port_name) !=
                        module_ptr->_axis_slave_ports.end());
  bool is_aximm_slave = (module_ptr->_aximm_slave_ports.find(port_name) !=
                         module_ptr->_aximm_slave_ports.end());
  if (!is_axis_slave && !is_aximm_slave) {
    bool is_axis_master = module_ptr->_axis_master_ports.find(port_name) !=
                          module_ptr->_axis_master_ports.end();
    bool is_aximm_master = module_ptr->_aximm_master_ports.find(port_name) !=
                           module_ptr->_aximm_master_ports.end();
    assert(is_aximm_master || is_axis_master);
  }
  return (is_axis_slave || is_aximm_slave);
}

std::string GetModuleNameFromPortName(std::string &port_name) {
  std::stringstream ss;
  ss.str(port_name);
  std::string module_name;
  std::getline(ss, module_name, '.');
  return module_name;
}

uint64_t DeterminedBaseAddress(int noc_id, int node_id) {
  int num_nocs = radsim_config.GetIntKnob("noc_num_nocs");
  int max_num_nodes = 0;
  for (int noc_id = 0; noc_id < num_nocs; noc_id++) {
    int num_nodes = radsim_config.GetIntVectorKnob("noc_num_nodes", noc_id);
    if (num_nodes > max_num_nodes) {
      max_num_nodes = num_nodes;
    }
  }
  int noc_bits = (int)ceil(log2(num_nocs));
  int node_bits = (int)ceil(log2(max_num_nodes));
  uint64_t base_addr = noc_id << node_bits;
  base_addr += node_id;
  base_addr <<= (AXI4_ADDRW - noc_bits - node_bits);
  return base_addr;
}

void RADSimDesignContext::ParseNoCPlacement(
    const std::string &placement_filename) {
  std::string placement_filepath =
      radsim_config.GetStringKnob("radsim_user_design_root_dir") + "/" +
      placement_filename;
  std::ifstream placement_file(placement_filepath);

  std::string line;
  while (std::getline(placement_file, line)) {
    std::stringstream ss(line);
    if (line.find('.') != std::string::npos) {
      std::string port_name, port_noc_placement_str, port_node_placement_str,
          port_axi_type;
      std::getline(ss, port_name, ' ');
      std::getline(ss, port_noc_placement_str, ' ');
      std::getline(ss, port_node_placement_str, ' ');
      std::getline(ss, port_axi_type, ' ');

      std::stringstream port_name_ss;
      port_name_ss << port_name;
      std::string module_name;
      std::getline(port_name_ss, module_name, '.');
      int port_noc_placement = std::stoi(port_noc_placement_str);
      int port_node_placement = std::stoi(port_node_placement_str);
      _port_placement[port_name] =
          std::make_tuple(port_noc_placement, port_node_placement, 0);

      // If port is AXI-S, we add it to the list of existing ports
      if (port_axi_type == "axis") {
        // Make sure that the same node was not used for an AXI-MM port
        assert(
            (_node_id_is_aximm[port_noc_placement].find(port_node_placement) ==
             _node_id_is_aximm[port_noc_placement].end()) ||
            (_node_id_is_aximm[port_noc_placement][port_node_placement] ==
             false));

        // If it is valid, add the port to the list of ports of this node
        if (_node_id_ports_list[port_noc_placement].find(port_node_placement) ==
            _node_id_ports_list[port_noc_placement].end()) {
          std::vector<std::string> port_list;
          port_list.push_back(port_name);
          _node_id_ports_list[port_noc_placement][port_node_placement] =
              port_list;
        } else {
          _node_id_ports_list[port_noc_placement][port_node_placement]
              .push_back(port_name);
        }
        _node_module_names[port_noc_placement][port_node_placement].insert(
            module_name);
        _node_id_is_aximm[port_noc_placement][port_node_placement] = false;
      } else {
        // Make sure the same node wasn't used for an AXI-MM or AXI-S port
        assert(
            _node_id_is_aximm[port_noc_placement].find(port_node_placement) ==
            _node_id_is_aximm[port_noc_placement].end());

        // If valid, add the port as the only port of this node
        std::vector<std::string> port_list;
        port_list.push_back(port_name);
        _node_id_ports_list[port_noc_placement][port_node_placement] =
            port_list;
        _node_id_is_aximm[port_noc_placement][port_node_placement] = true;
        _node_module_names[port_noc_placement][port_node_placement].insert(
            module_name);

        // Set base address information
        _aximm_port_base_addresses[port_name] =
            DeterminedBaseAddress(port_noc_placement, port_node_placement);
      }
    } else {
      std::string module_name, port_name, port_noc_placement_str,
          port_node_placement_str, port_axi_type;
      std::getline(ss, module_name, ' ');
      std::getline(ss, port_noc_placement_str, ' ');
      std::getline(ss, port_node_placement_str, ' ');
      std::getline(ss, port_axi_type, '\n');

      int port_noc_placement = std::stoi(port_noc_placement_str);
      int port_node_placement = std::stoi(port_node_placement_str);

      // Ensure the module has been instantiated
      if (_design_modules.find(module_name) == _design_modules.end())
        sim_log.log(error, "NoC module " + module_name + " is not defined!");

      RADSimModule *module_ptr = _design_modules[module_name];

      if (port_axi_type == "axis") {
        // Loop over all the slave ports of this module
        for (unsigned int port_id = 0;
             port_id < module_ptr->_ordered_axis_slave_ports.size();
             port_id++) {
          port_name = module_ptr->_ordered_axis_slave_ports[port_id];
          _port_placement[port_name] =
              std::make_tuple(port_noc_placement, port_node_placement, 0);

          // Make sure that the same node was not used for an AXI-MM port
          assert((_node_id_is_aximm[port_noc_placement].find(
                      port_node_placement) ==
                  _node_id_is_aximm[port_noc_placement].end()) ||
                 !_node_id_is_aximm[port_noc_placement][port_node_placement]);

          // If it is valid, add the port to the list of ports of this node
          if (_node_id_ports_list[port_noc_placement].find(
                  port_node_placement) ==
              _node_id_ports_list[port_noc_placement].end()) {
            std::vector<std::string> port_list;
            port_list.push_back(port_name);
            _node_id_ports_list[port_noc_placement][port_node_placement] =
                port_list;
          } else {
            _node_id_ports_list[port_noc_placement][port_node_placement]
                .push_back(port_name);
          }
        }

        // Loop over all the master ports of this module
        for (unsigned int port_id = 0;
             port_id < module_ptr->_ordered_axis_master_ports.size();
             port_id++) {
          port_name = module_ptr->_ordered_axis_master_ports[port_id];
          _port_placement[port_name] =
              std::make_tuple(port_noc_placement, port_node_placement, 0);

          // Make sure that the same node was not used for an AXI-MM port
          assert((_node_id_is_aximm[port_noc_placement].find(
                      port_node_placement) ==
                  _node_id_is_aximm[port_noc_placement].end()) ||
                 !_node_id_is_aximm[port_noc_placement][port_node_placement]);

          // If it is valid, add the port to the list of ports of this node
          if (_node_id_ports_list[port_noc_placement].find(
                  port_node_placement) ==
              _node_id_ports_list[port_noc_placement].end()) {
            std::vector<std::string> port_list;
            port_list.push_back(port_name);
            _node_id_ports_list[port_noc_placement][port_node_placement] =
                port_list;
          } else {
            _node_id_ports_list[port_noc_placement][port_node_placement]
                .push_back(port_name);
          }
        }
        _node_id_is_aximm[port_noc_placement][port_node_placement] = false;
      } else {
        for (unsigned int port_id = 0;
             port_id < module_ptr->_ordered_aximm_slave_ports.size();
             port_id++) {
          port_name = module_ptr->_ordered_aximm_slave_ports[0];
          _port_placement[port_name] =
              std::make_tuple(port_noc_placement, port_node_placement, 0);

          // Make sure the same node wasn't used for an AXI-MM or AXI-S port
          assert(
              _node_id_is_aximm[port_noc_placement].find(port_node_placement) ==
              _node_id_is_aximm[port_noc_placement].end());

          // If valid, add the port as the only port of this node
          if (_node_id_ports_list[port_noc_placement].find(
                  port_node_placement) ==
              _node_id_ports_list[port_noc_placement].end()) {
            std::vector<std::string> port_list;
            port_list.push_back(port_name);
            _node_id_ports_list[port_noc_placement][port_node_placement] =
                port_list;
            _node_id_is_aximm[port_noc_placement][port_node_placement] = true;
          } else {
            _node_id_ports_list[port_noc_placement][port_node_placement]
                .push_back(port_name);
          }
          // Set base address information
          _aximm_port_base_addresses[port_name] =
              DeterminedBaseAddress(port_noc_placement, port_node_placement);
        }

        for (unsigned int port_id = 0;
             port_id < module_ptr->_ordered_aximm_master_ports.size();
             port_id++) {
          port_name = module_ptr->_ordered_aximm_master_ports[0];
          _port_placement[port_name] =
              std::make_tuple(port_noc_placement, port_node_placement, 0);

          // Make sure the same node wasn't used for an AXI-MM or AXI-S port
          assert(
              _node_id_is_aximm[port_noc_placement].find(port_node_placement) ==
              _node_id_is_aximm[port_noc_placement].end());

          // If valid, add the port as the only port of this node
          if (_node_id_ports_list[port_noc_placement].find(
                  port_node_placement) ==
              _node_id_ports_list[port_noc_placement].end()) {
            std::vector<std::string> port_list;
            port_list.push_back(port_name);
            _node_id_ports_list[port_noc_placement][port_node_placement] =
                port_list;
            _node_id_is_aximm[port_noc_placement][port_node_placement] = true;
          } else {
            _node_id_ports_list[port_noc_placement][port_node_placement]
                .push_back(port_name);
          }
          // Set base address information
          _aximm_port_base_addresses[port_name] =
              DeterminedBaseAddress(port_noc_placement, port_node_placement);
        }
      }
      _node_module_names[port_noc_placement][port_node_placement].insert(
          module_name);
    }
  }
}

void RADSimDesignContext::ParseClockSettings(const std::string &clks_filename) {
  std::string clks_filepath =
      radsim_config.GetStringKnob("radsim_user_design_root_dir") + "/" +
      clks_filename;
  std::ifstream clks_file(clks_filepath);

  std::string line;
  while (std::getline(clks_file, line)) {
    std::stringstream ss(line);

    std::string module_name, adapter_clk_str, module_clk_str;
    std::getline(ss, module_name, ' ');
    std::getline(ss, adapter_clk_str, ' ');
    std::getline(ss, module_clk_str, ' ');

    int adapter_clk_idx = std::stoi(adapter_clk_str);
    int module_clk_idx = std::stoi(module_clk_str);

    _module_clk_settings[module_name] =
        std::make_pair(adapter_clk_idx, module_clk_idx);
  }
}

void RADSimDesignContext::RegisterModule(std::string module_name,
                                         RADSimModule *module_ptr) {
  if (_design_modules.find(module_name) != _design_modules.end())
    sim_log.log(error, "Module with the same name \"" + module_name +
                           "\" already exists!");
  _design_modules[module_name] = module_ptr;
}

void RADSimDesignContext::BuildDesignContext(
    const std::string &placement_filename, const std::string &clks_filename) {
  unsigned int num_nocs = radsim_config.GetIntKnob("noc_num_nocs");
  _node_id_is_aximm.resize(num_nocs);
  _node_id_ports_list.resize(num_nocs);
  _noc_axis_slave_adapter_info.resize(num_nocs);
  _noc_axis_master_adapter_info.resize(num_nocs);
  _noc_aximm_slave_adapter_info.resize(num_nocs);
  _noc_aximm_master_adapter_info.resize(num_nocs);
  _noc_axis_slave_ports.resize(num_nocs);
  _noc_axis_master_ports.resize(num_nocs);
  _num_noc_axis_slave_ports.resize(num_nocs);
  _num_noc_axis_master_ports.resize(num_nocs);
  _noc_aximm_slave_ports.resize(num_nocs);
  _noc_aximm_master_ports.resize(num_nocs);
  _num_noc_aximm_slave_ports.resize(num_nocs);
  _num_noc_aximm_master_ports.resize(num_nocs);

  ParseNoCPlacement(placement_filename);
  ParseClockSettings(clks_filename);

  for (unsigned int noc_id = 0; noc_id < num_nocs; noc_id++) {
    for (auto node_it = _node_id_ports_list[noc_id].begin();
         node_it != _node_id_ports_list[noc_id].end(); node_it++) {
      unsigned int node_id = node_it->first;
      std::vector<std::string> port_list = node_it->second;
      std::string module_name = GetModuleNameFromPortName(port_list[0]);
      unsigned int adapter_clk_idx = _module_clk_settings[module_name].first;
      unsigned int module_clk_idx = _module_clk_settings[module_name].second;

      std::vector<std::string> slave_port_names, master_port_names;
      std::vector<unsigned int> slave_port_types, master_port_types,
          slave_port_dataw, master_port_dataw;
      std::vector<bool> slave_port_is_aximm, master_port_is_aximm;
      unsigned int slave_port_id = 0, master_port_id = 0;

      for (unsigned int port_id = 0; port_id < port_list.size(); port_id++) {
        std::string port_name = port_list[port_id];
        module_name = GetModuleNameFromPortName(port_name);
        RADSimModule *module_ptr = _design_modules[module_name];
        bool is_slave = IsSlavePort(port_name, module_ptr);
        if (!is_slave) {
          slave_port_names.push_back(port_name);
          slave_port_types.push_back(module_ptr->_ports_types[port_name]);
          slave_port_dataw.push_back(module_ptr->_ports_dataw[port_name]);
          slave_port_is_aximm.push_back(module_ptr->_ports_is_aximm[port_name]);
          std::get<2>(_port_placement[port_name]) = slave_port_id;
          slave_port_id++;
        } else {
          master_port_names.push_back(port_name);
          master_port_types.push_back(module_ptr->_ports_types[port_name]);
          master_port_dataw.push_back(module_ptr->_ports_dataw[port_name]);
          master_port_is_aximm.push_back(
              module_ptr->_ports_is_aximm[port_name]);
          std::get<2>(_port_placement[port_name]) = master_port_id;
          master_port_id++;
        }
      }

      if (slave_port_id > 0) {
        AdapterInfo slave_adapter;
        slave_adapter._noc_id = noc_id;
        slave_adapter._node_id = node_id;
        slave_adapter._adapter_clk_idx = adapter_clk_idx;
        slave_adapter._module_clk_idx = module_clk_idx;
        slave_adapter._port_names = slave_port_names;
        slave_adapter._port_types = slave_port_types;
        slave_adapter._port_dataw = slave_port_dataw;
        slave_adapter._is_aximm = slave_port_is_aximm[0];

        if (!slave_port_is_aximm[0]) {
          _num_noc_axis_slave_ports[noc_id].push_back(slave_port_id);
          _noc_axis_slave_adapter_info[noc_id].push_back(slave_adapter);
        } else {
          assert(slave_port_id == 1);
          _num_noc_aximm_slave_ports[noc_id].push_back(slave_port_id);
          _noc_aximm_slave_adapter_info[noc_id].push_back(slave_adapter);
        }
      }

      if (master_port_id > 0) {
        AdapterInfo master_adapter;
        master_adapter._noc_id = noc_id;
        master_adapter._node_id = node_id;
        master_adapter._adapter_clk_idx = adapter_clk_idx;
        master_adapter._module_clk_idx = module_clk_idx;
        master_adapter._port_names = master_port_names;
        master_adapter._port_types = master_port_types;
        master_adapter._port_dataw = master_port_dataw;
        master_adapter._is_aximm = master_port_is_aximm[0];

        if (!master_port_is_aximm[0]) {
          _num_noc_axis_master_ports[noc_id].push_back(master_port_id);
          _noc_axis_master_adapter_info[noc_id].push_back(master_adapter);
        } else {
          assert(master_port_id == 1);
          _num_noc_aximm_master_ports[noc_id].push_back(master_port_id);
          _noc_aximm_master_adapter_info[noc_id].push_back(master_adapter);
        }
      }
    }
  }

  for (auto module_it = _design_modules.begin();
       module_it != _design_modules.end(); module_it++) {
    std::string module_name = module_it->first;
    RADSimModule *module_ptr = module_it->second;
    unsigned int module_clk_idx = _module_clk_settings[module_name].second;
    module_ptr->clk(*_module_clks[module_clk_idx]);
  }

  unsigned int axis_signal_count = 0;
  for (unsigned int noc_id = 0; noc_id < num_nocs; noc_id++) {
    for (unsigned int port_id = 0;
         port_id < _num_noc_axis_slave_ports[noc_id].size(); port_id++)
      axis_signal_count += _num_noc_axis_slave_ports[noc_id][port_id];
    for (unsigned int port_id = 0;
         port_id < _num_noc_axis_master_ports[noc_id].size(); port_id++)
      axis_signal_count += _num_noc_axis_master_ports[noc_id][port_id];
  }
  _axis_signals.resize(axis_signal_count);

  unsigned int aximm_signal_count = 0;
  for (unsigned int noc_id = 0; noc_id < num_nocs; noc_id++) {
    for (unsigned int port_id = 0;
         port_id < _num_noc_aximm_slave_ports[noc_id].size(); port_id++) {
      aximm_signal_count += 1;
    }
    for (unsigned int port_id = 0;
         port_id < _num_noc_aximm_master_ports[noc_id].size(); port_id++) {
      aximm_signal_count += 1;
    }
  }
  _aximm_signals.resize(aximm_signal_count);
}

void RADSimDesignContext::CreateSystemNoCs(sc_in<bool> &rst) {
  unsigned int num_nocs = radsim_config.GetIntKnob("noc_num_nocs");
  for (unsigned int noc_id = 0; noc_id < num_nocs; noc_id++) {
    std::string noc_name_str = "radsim_noc_" + std::to_string(noc_id);
    const char *noc_name = noc_name_str.c_str();
    radsim_noc *noc_inst =
        new radsim_noc(noc_name, noc_id, _adapter_clks, _module_clks,
                       _noc_axis_master_adapter_info[noc_id],
                       _noc_axis_slave_adapter_info[noc_id],
                       _noc_aximm_master_adapter_info[noc_id],
                       _noc_aximm_slave_adapter_info[noc_id]);
    noc_inst->noc_clk(*_noc_clks[noc_id]);
    noc_inst->rst(rst);

    _nocs.push_back(noc_inst);
  }
}

void RADSimDesignContext::ConnectModulesToNoC() {
  unsigned int axis_signal_id = 0;
  unsigned int aximm_signal_id = 0;

  for (auto module_it = _design_modules.begin();
       module_it != _design_modules.end(); module_it++) {
    RADSimModule *module_ptr = module_it->second;
    // std::cout << "MODULE " << module_ptr->name() << std::endl;

    // Connect AXI-S Slave ports of the module
    // std::cout << "AXI-S slave ports: " << std::endl;
    for (auto slave_port_it = module_ptr->_axis_slave_ports.begin();
         slave_port_it != module_ptr->_axis_slave_ports.end();
         slave_port_it++) {
      std::string port_name = slave_port_it->first;
      // std::cout << port_name << ", ";
      if (_port_placement.find(port_name) == _port_placement.end())
        sim_log.log(error, "Port " + port_name + " has no NoC placement defined!");
      unsigned int noc_id = std::get<0>(_port_placement[port_name]);
      _axis_signals[axis_signal_id].Connect(
          *(_noc_axis_master_ports[noc_id][port_name]),
          *(slave_port_it->second));
      axis_signal_id++;
    }

    // Connect AXI-S Master ports of the module
    // std::cout << "\nAXI-S master ports: ";
    for (auto master_port_it = module_ptr->_axis_master_ports.begin();
         master_port_it != module_ptr->_axis_master_ports.end();
         master_port_it++) {
      std::string port_name = master_port_it->first;
      // std::cout << port_name << ", ";
      if (_port_placement.find(port_name) == _port_placement.end())
        sim_log.log(error, "Port " + port_name + " has no NoC placement defined!");
      unsigned int noc_id = std::get<0>(_port_placement[port_name]);
      _axis_signals[axis_signal_id].Connect(
          *(master_port_it->second),
          *(_noc_axis_slave_ports[noc_id][port_name]));
      axis_signal_id++;
    }

    // Connect AXI-MM Slave ports of the module
    // std::cout << "\nAXI-MM slave ports: ";
    for (auto slave_port_it = module_ptr->_aximm_slave_ports.begin();
         slave_port_it != module_ptr->_aximm_slave_ports.end();
         slave_port_it++) {
      std::string port_name = slave_port_it->first;
      // std::cout << port_name << ", ";
      if (_port_placement.find(port_name) == _port_placement.end())
        sim_log.log(error, "Port " + port_name + " has no NoC placement defined!");
      unsigned int noc_id = std::get<0>(_port_placement[port_name]);
      _aximm_signals[aximm_signal_id].Connect(
          *(_noc_aximm_master_ports[noc_id][port_name]),
          *(slave_port_it->second));
      aximm_signal_id++;
    }

    // Connect AXI-MM Master ports of the module
    // std::cout << "\nAXI-MM master ports: ";
    for (auto master_port_it = module_ptr->_aximm_master_ports.begin();
         master_port_it != module_ptr->_aximm_master_ports.end();
         master_port_it++) {
      std::string port_name = master_port_it->first;
      // std::cout << port_name << ", ";
      if (_port_placement.find(port_name) == _port_placement.end())
        sim_log.log(error, "Port " + port_name + " has no NoC placement defined!");
      unsigned int noc_id = std::get<0>(_port_placement[port_name]);
      _aximm_signals[aximm_signal_id].Connect(
          *(master_port_it->second),
          *(_noc_aximm_slave_ports[noc_id][port_name]));
      aximm_signal_id++;
    }
    // std::cout << "\n";
  }
}

void RADSimDesignContext::RegisterNoCSlavePort(unsigned int noc_id,
                                               std::string &port_name,
                                               axis_slave_port *port_ptr) {
  _noc_axis_slave_ports[noc_id][port_name] = port_ptr;
}

void RADSimDesignContext::RegisterNoCMasterPort(unsigned int noc_id,
                                                std::string &port_name,
                                                axis_master_port *port_ptr) {
  _noc_axis_master_ports[noc_id][port_name] = port_ptr;
}

void RADSimDesignContext::RegisterNoCSlavePort(unsigned int noc_id,
                                               std::string &port_name,
                                               aximm_slave_port *port_ptr) {
  _noc_aximm_slave_ports[noc_id][port_name] = port_ptr;
}

void RADSimDesignContext::RegisterNoCMasterPort(unsigned int noc_id,
                                                std::string &port_name,
                                                aximm_master_port *port_ptr) {
  _noc_aximm_master_ports[noc_id][port_name] = port_ptr;
}

axis_slave_port *
RADSimDesignContext::GetNoCAxisSlavePort(unsigned int noc_id,
                                         std::string &port_name) {
  if (_noc_axis_slave_ports[noc_id].find(port_name) ==
      _noc_axis_slave_ports[noc_id].end()) {
    std::cerr << "Cannot find " << port_name << " in list of slave NoC ports"
              << std::endl;
    exit(1);
  }
  return _noc_axis_slave_ports[noc_id][port_name];
}

axis_master_port *
RADSimDesignContext::GetNoCAxisMasterPort(unsigned int noc_id,
                                          std::string &port_name) {
  if (_noc_axis_master_ports[noc_id].find(port_name) ==
      _noc_axis_master_ports[noc_id].end()) {
    std::cerr << "Cannot find " << port_name << " in list of master NoC ports"
              << std::endl;
    exit(1);
  }
  return _noc_axis_master_ports[noc_id][port_name];
}

unsigned int RADSimDesignContext::GetNumNoCSlaveAdapters(unsigned int noc_id,
                                                         bool is_aximm) {
  if (is_aximm) {
    return _num_noc_aximm_slave_ports[noc_id].size();
  } else {
    return _num_noc_axis_slave_ports[noc_id].size();
  }
}

unsigned int RADSimDesignContext::GetNumNoCMasterAdapters(unsigned int noc_id,
                                                          bool is_aximm) {
  if (is_aximm) {
    return _num_noc_aximm_master_ports[noc_id].size();
  } else {
    return _num_noc_axis_master_ports[noc_id].size();
  }
}

unsigned int
RADSimDesignContext::GetNumAxisSlaveAdapterPorts(unsigned int noc_id,
                                                 unsigned int adapter_id) {
  if (adapter_id >= _num_noc_axis_slave_ports[noc_id].size()) {
    std::cerr << "Requested adapter ID (" << adapter_id
              << ") exceeds the number of total slave adapters ("
              << _num_noc_axis_slave_ports[noc_id].size() << ") in this NoC!"
              << std::endl;
    exit(1);
  }
  return _num_noc_axis_slave_ports[noc_id][adapter_id];
}

unsigned int
RADSimDesignContext::GetNumAxisMasterAdapterPorts(unsigned int noc_id,
                                                  unsigned int adapter_id) {
  if (adapter_id >= _num_noc_axis_master_ports[noc_id].size()) {
    std::cerr << "Requested adapter ID (" << adapter_id
              << ") exceeds the number of total master adapters ("
              << _num_noc_axis_master_ports[noc_id].size() << ") in this NoC!"
              << std::endl;
    exit(1);
  }
  return _num_noc_axis_master_ports[noc_id][adapter_id];
}

std::tuple<unsigned int, unsigned int, unsigned int>
RADSimDesignContext::GetPortPlacement(std::string &port_name) {
  if (_port_placement.find(port_name) == _port_placement.end())
    sim_log.log(error, "Port " + port_name + " has no NoC placement defined!");
  return _port_placement[port_name];
}

unsigned int RADSimDesignContext::GetPortDestinationID(std::string &port_name) {
  if (_port_placement.find(port_name) == _port_placement.end())
    sim_log.log(error, "Port " + port_name + " has no NoC placement defined!");
  return std::get<1>(_port_placement[port_name]);
}

unsigned int RADSimDesignContext::GetPortInterfaceID(std::string &port_name) {
  if (_port_placement.find(port_name) == _port_placement.end())
    sim_log.log(error, "Port " + port_name + " has no NoC placement defined!");
  return std::get<2>(_port_placement[port_name]);
}

void RADSimDesignContext::DumpDesignContext() {
  std::cout << "PORT PLACEMENT: " << std::endl;
  for (auto it = _port_placement.begin(); it != _port_placement.end(); it++) {
    std::cout << it->first << ": ";
    std::tuple<unsigned int, unsigned int, unsigned int> p = it->second;
    std::cout << std::get<1>(p) << " " << std::get<2>(p) << std::endl;
  }
  std::cout << "**************************" << std::endl;
  cin.get();
}

std::vector<std::vector<std::set<std::string>>> &
RADSimDesignContext::GetNodeModuleNames() {
  return _node_module_names;
}

uint64_t RADSimDesignContext::GetPortBaseAddress(std::string &port_name) {
  assert(_aximm_port_base_addresses.find(port_name) !=
         _aximm_port_base_addresses.end());
  return _aximm_port_base_addresses[port_name];
}

int RADSimDesignContext::GetDesignResult() {
  return _design_result;
}

void RADSimDesignContext::ReportDesignFailure() {
  _design_result = 1;
}