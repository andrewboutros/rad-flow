#include <design_context.hpp>
#include <radsim_noc.hpp>

radsim_noc::radsim_noc(const sc_module_name &name, int noc_id,
                       std::vector<sc_clock *> &adapter_clks,
                       std::vector<sc_clock *> &module_clks,
                       std::vector<AdapterInfo> &axis_master_adapter_info,
                       std::vector<AdapterInfo> &axis_slave_adapter_info,
                       std::vector<AdapterInfo> &aximm_master_adapter_info,
                       std::vector<AdapterInfo> &aximm_slave_adapter_info)
    : sc_module(name), noc_clk("noc_clk"), rst("rst") {

  _noc_id = noc_id;
  _num_noc_nodes = radsim_config.GetIntVectorKnob("noc_num_nodes", _noc_id);

  // Parse config file, initialize routing data structures and create Booksim
  // NoC
  std::string config_filename = radsim_config.GetStringKnob("radsim_root_dir") +
                                "/sim/noc/noc" + std::to_string(noc_id) +
                                "_config";
  _config.ParseFile(config_filename);
  InitializeRoutingMap(_config);
  _booksim_noc = Network::New(_config, "noc_" + to_string(_noc_id));

  // Set routing-related specifications
  string rf =
      _config.GetStr("routing_function") + "_" + _config.GetStr("topology");
  map<string, tRoutingFunction>::const_iterator rf_iter =
      gRoutingFunctionMap.find(rf);
  if (rf_iter == gRoutingFunctionMap.end()) {
    cerr << "Invalid routing function: " + rf << endl;
  }
  _routing_func = rf_iter->second;
  _lookahead_routing = !_config.GetInt("routing_delay");
  _wait_for_tail_credit = _config.GetInt("wait_for_tail_credit");

  // Create ejected flits map and buffer states for all routers in all subnets
  _ejected_flits = new map<int, int>();
  _buffer_state.resize(_num_noc_nodes);
  for (int node_id = 0; node_id < _num_noc_nodes; node_id++) {
    std::ostringstream tmp_name;
    tmp_name << "terminal_buf_state_" << node_id << "_" << _noc_id;
    auto *bs = new BufferState(_config, nullptr, tmp_name.str());
    int vc_alloc_delay = _config.GetInt("vc_alloc_delay");
    int sw_alloc_delay = _config.GetInt("sw_alloc_delay");
    int router_latency =
        _config.GetInt("routing_delay") +
        (_config.GetInt("speculative") ? max(vc_alloc_delay, sw_alloc_delay)
                                       : (vc_alloc_delay + sw_alloc_delay));
    int min_latency = 1 + _booksim_noc->GetInject(node_id)->GetLatency() +
                      router_latency +
                      _booksim_noc->GetInjectCred(node_id)->GetLatency();
    bs->SetMinLatency(min_latency);
    _buffer_state[node_id] = bs;
  }

  // Create NoC AXI-S Master adapters
  _num_axis_master_endpoints =
      radsim_design.GetNumNoCMasterAdapters(_noc_id, false);
  noc_axis_master_ports.init(_num_axis_master_endpoints);
  for (unsigned int adapter_id = 0; adapter_id < _num_axis_master_endpoints;
       adapter_id++) {
    unsigned int num_adapter_ports =
        radsim_design.GetNumAxisMasterAdapterPorts(_noc_id, adapter_id);
    noc_axis_master_ports[adapter_id].init(num_adapter_ports);

    // Prepare adapter information
    std::string adapter_name_str =
        "axis_master_adapter_" + std::to_string(adapter_id);
    const char *adapter_name = adapter_name_str.c_str();
    std::vector<Flit::FlitType> adapter_port_types;
    for (auto it = axis_master_adapter_info[adapter_id]._port_types.begin();
         it != axis_master_adapter_info[adapter_id]._port_types.end(); it++)
      adapter_port_types.push_back(static_cast<Flit::FlitType>(*it));

    // Create adapter
    axis_master_adapter *master_adapter = new axis_master_adapter(
        adapter_name, axis_master_adapter_info[adapter_id]._node_id, _noc_id,
        adapter_port_types, axis_master_adapter_info[adapter_id]._port_dataw,
        &_config, _booksim_noc,
        _buffer_state[axis_master_adapter_info[adapter_id]._node_id],
        _routing_func, _lookahead_routing, _wait_for_tail_credit,
        _ejected_flits);

    // Connect adapter ports and register NoC port in design context
    master_adapter->noc_clk(noc_clk);
    master_adapter->node_clk(
        *module_clks[axis_master_adapter_info[adapter_id]._module_clk_idx]);
    master_adapter->adapter_clk(
        *adapter_clks[axis_master_adapter_info[adapter_id]._adapter_clk_idx]);
    master_adapter->rst(rst);
    for (unsigned int port_id = 0; port_id < num_adapter_ports; port_id++) {
      std::string port_name =
          axis_master_adapter_info[adapter_id]._port_names[port_id];
      master_adapter->axis_interfaces[port_id].ConnectToPort(
          noc_axis_master_ports[adapter_id][port_id]);
      radsim_design.RegisterNoCMasterPort(
          _noc_id, port_name, &noc_axis_master_ports[adapter_id][port_id]);
    }

    _axis_master_adapters.push_back(master_adapter);
  }

  // Create NoC AXI-S Slave adapters
  _num_axis_slave_endpoints =
      radsim_design.GetNumNoCSlaveAdapters(_noc_id, false);
  noc_axis_slave_ports.init(_num_axis_slave_endpoints);
  for (unsigned int adapter_id = 0; adapter_id < _num_axis_slave_endpoints;
       adapter_id++) {
    unsigned int num_adapter_ports =
        radsim_design.GetNumAxisSlaveAdapterPorts(_noc_id, adapter_id);
    noc_axis_slave_ports[adapter_id].init(num_adapter_ports);

    // Prepare adapter information
    std::string adapter_name_str =
        "axis_slave_adapter_" + std::to_string(adapter_id);
    const char *adapter_name = adapter_name_str.c_str();
    std::vector<Flit::FlitType> adapter_port_types;
    for (auto it = axis_slave_adapter_info[adapter_id]._port_types.begin();
         it != axis_slave_adapter_info[adapter_id]._port_types.end(); it++)
      adapter_port_types.push_back(static_cast<Flit::FlitType>(*it));
    double adapter_module_period = radsim_config.GetDoubleVectorKnob(
        "design_clk_periods", axis_slave_adapter_info[adapter_id]._module_clk_idx);
    double adapter_period = radsim_config.GetDoubleVectorKnob(
        "noc_adapters_clk_period", axis_slave_adapter_info[adapter_id]._adapter_clk_idx);

    // Create adapter
    axis_slave_adapter *slave_adapter = new axis_slave_adapter(
        adapter_name, axis_slave_adapter_info[adapter_id]._node_id, _noc_id,
        adapter_port_types, axis_slave_adapter_info[adapter_id]._port_dataw,
        adapter_module_period, adapter_period, &_config, _booksim_noc,
        _buffer_state[axis_slave_adapter_info[adapter_id]._node_id],
        _routing_func, _lookahead_routing, _wait_for_tail_credit);

    // Connect adapter ports and register NoC port in design context
    slave_adapter->noc_clk(noc_clk);
    slave_adapter->node_clk(
        *module_clks[axis_slave_adapter_info[adapter_id]._module_clk_idx]);
    slave_adapter->adapter_clk(
        *adapter_clks[axis_slave_adapter_info[adapter_id]._adapter_clk_idx]);
    slave_adapter->rst(rst);
    for (unsigned int port_id = 0; port_id < num_adapter_ports; port_id++) {
      std::string port_name =
          axis_slave_adapter_info[adapter_id]._port_names[port_id];
      slave_adapter->axis_interfaces[port_id].ConnectToPort(
          noc_axis_slave_ports[adapter_id][port_id]);
      radsim_design.RegisterNoCSlavePort(
          _noc_id, port_name, &noc_axis_slave_ports[adapter_id][port_id]);
    }

    _axis_slave_adapters.push_back(slave_adapter);
  }

  // Create NoC AXI-MM Master adapters
  _num_aximm_slave_endpoints =
      radsim_design.GetNumNoCMasterAdapters(_noc_id, true);
  noc_aximm_master_ports.init(_num_aximm_slave_endpoints);
  for (unsigned int adapter_id = 0; adapter_id < _num_aximm_slave_endpoints;
       adapter_id++) {

    // Prepare adapter information
    std::string adapter_name_str =
        "aximm_master_adapter_" + std::to_string(adapter_id);
    const char *adapter_name = adapter_name_str.c_str();
    double adapter_module_period = radsim_config.GetDoubleVectorKnob(
        "design_clk_periods", aximm_master_adapter_info[adapter_id]._module_clk_idx);
    double adapter_period = radsim_config.GetDoubleVectorKnob(
        "noc_adapters_clk_period",
        aximm_master_adapter_info[adapter_id]._adapter_clk_idx);

    // Create adapter
    aximm_master_adapter *master_adapter = new aximm_master_adapter(
        adapter_name, aximm_master_adapter_info[adapter_id]._node_id, _noc_id,
        &_config, _booksim_noc,
        _buffer_state[aximm_master_adapter_info[adapter_id]._node_id],
        _routing_func, _lookahead_routing, _wait_for_tail_credit,
        _ejected_flits, aximm_master_adapter_info[adapter_id]._port_dataw[0],
        adapter_module_period, adapter_period);

    // Connect adapter ports and register NoC port in design context
    master_adapter->noc_clk(noc_clk);
    master_adapter->node_clk(
        *module_clks[aximm_master_adapter_info[adapter_id]._module_clk_idx]);
    master_adapter->adapter_clk(
        *adapter_clks[aximm_master_adapter_info[adapter_id]._adapter_clk_idx]);
    master_adapter->rst(rst);
    master_adapter->aximm_interface.ConnectToPort(
        noc_aximm_master_ports[adapter_id]);
    std::string port_name =
        aximm_master_adapter_info[adapter_id]._port_names[0];
    radsim_design.RegisterNoCMasterPort(_noc_id, port_name,
                                        &noc_aximm_master_ports[adapter_id]);

    _aximm_master_adapters.push_back(master_adapter);
  }

  // Create NoC AXI-MM Slave adapters
  _num_aximm_master_endpoints =
      radsim_design.GetNumNoCSlaveAdapters(_noc_id, true);
  noc_aximm_slave_ports.init(_num_aximm_master_endpoints);
  for (unsigned int adapter_id = 0; adapter_id < _num_aximm_master_endpoints;
       adapter_id++) {

    // Prepare adapter information
    std::string adapter_name_str =
        "aximm_slave_adapter_" + std::to_string(adapter_id);
    const char *adapter_name = adapter_name_str.c_str();
    double adapter_module_period = radsim_config.GetDoubleVectorKnob(
        "design_clk_periods", aximm_slave_adapter_info[adapter_id]._module_clk_idx);
    double adapter_period = radsim_config.GetDoubleVectorKnob(
        "noc_adapters_clk_period",
        aximm_slave_adapter_info[adapter_id]._adapter_clk_idx);

    // Create adapter
    aximm_slave_adapter *slave_adapter = new aximm_slave_adapter(
        adapter_name, aximm_slave_adapter_info[adapter_id]._node_id, _noc_id,
        &_config, _booksim_noc,
        _buffer_state[aximm_slave_adapter_info[adapter_id]._node_id],
        _routing_func, _lookahead_routing, _wait_for_tail_credit,
        _ejected_flits, aximm_slave_adapter_info[adapter_id]._port_dataw[0],
        adapter_module_period, adapter_period);

    // Connect adapter ports and register NoC port in design context
    slave_adapter->noc_clk(noc_clk);
    slave_adapter->node_clk(
        *module_clks[aximm_slave_adapter_info[adapter_id]._module_clk_idx]);
    slave_adapter->adapter_clk(
        *adapter_clks[aximm_slave_adapter_info[adapter_id]._adapter_clk_idx]);
    slave_adapter->rst(rst);
    slave_adapter->aximm_interface.ConnectToPort(
        noc_aximm_slave_ports[adapter_id]);
    std::string port_name = aximm_slave_adapter_info[adapter_id]._port_names[0];
    radsim_design.RegisterNoCSlavePort(_noc_id, port_name,
                                       &noc_aximm_slave_ports[adapter_id]);

    _aximm_slave_adapters.push_back(slave_adapter);
  }

  SC_CTHREAD(Tick, noc_clk.pos());
  reset_signal_is(rst, true);
}

radsim_noc::~radsim_noc() {
  delete _booksim_noc;
  delete _ejected_flits;
  for (int node_id = 0; node_id < _num_noc_nodes; node_id++) {
    delete _buffer_state[node_id];
  }

  for (unsigned int adapter_id = 0; adapter_id < _num_axis_slave_endpoints;
       adapter_id++)
    delete _axis_slave_adapters[adapter_id];
  for (unsigned int adapter_id = 0; adapter_id < _num_axis_master_endpoints;
       adapter_id++)
    delete _axis_master_adapters[adapter_id];

  for (unsigned int adapter_id = 0; adapter_id < _num_aximm_master_endpoints;
       adapter_id++)
    delete _aximm_slave_adapters[adapter_id];
  for (unsigned int adapter_id = 0; adapter_id < _num_aximm_slave_endpoints;
       adapter_id++)
    delete _aximm_master_adapters[adapter_id];
}

Network *radsim_noc::GetNetwork() { return _booksim_noc; }

BookSimConfig *radsim_noc::GetConfig() { return &_config; }

int radsim_noc::GetNumNodes() { return _booksim_noc->NumNodes(); }

tRoutingFunction radsim_noc::GetRoutingFunction() const {
  return _routing_func;
}

bool radsim_noc::IsLookaheadRouting() const { return _lookahead_routing; }

bool radsim_noc::IsWaitForTailCredit() const { return _wait_for_tail_credit; }

BufferState *radsim_noc::GetBufferState(int node) {
  assert(node < _num_noc_nodes);
  return _buffer_state[node];
}

map<int, int> *radsim_noc::GetEjectedFlitsMap() { return _ejected_flits; }

void radsim_noc::Tick() {
  // Reset Code
  wait();

  while (true) {
    _booksim_noc->ReadInputs();
    for (int node_id = 0; node_id < _num_noc_nodes; node_id++) {
      map<int, int>::const_iterator iter = _ejected_flits->find(node_id);
      if (iter != _ejected_flits->end()) {
        Credit *const c = Credit::New();
        c->vc.insert(iter->second);
        _booksim_noc->WriteCredit(c, node_id);
      }
    }
    _ejected_flits->clear();
    _booksim_noc->Evaluate();
    _booksim_noc->WriteOutputs();
    _booksim_noc->Trace(NoCFlitTelemetry::ostream, double(GetSimTime()) / 1000);
    wait();
  }
}