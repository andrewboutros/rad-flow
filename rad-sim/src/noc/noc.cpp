#include "noc.hpp"

noc::noc(const sc_module_name& name, string& config_filename, std::vector<sc_clock*>& node_clks,
         std::vector<int>& slave_adapter_node_ids,
         std::vector<std::vector<Flit::FlitType>>& slave_adapter_interface_types,
         std::vector<float>& slave_adapter_node_periods, std::vector<unsigned int>& slave_adapter_node_clks,
         std::vector<std::vector<unsigned int>>& slave_adapter_interface_dataw,
         std::vector<int>& master_adapter_node_ids,
         std::vector<std::vector<Flit::FlitType>>& master_adapter_interface_types,
         std::vector<std::vector<unsigned int>>& master_adapter_interface_dataw,
         std::vector<unsigned int>& master_adapter_node_clks)
    : sc_module(name),
      adapter_clk("adapter_clk"),
      noc_clk("noc_clk"),
      rst("rst"),
      noc_master_ports("noc_master_ports"),
      noc_slave_ports("noc_slave_ports") {
  // Parse config file and initialize routing data structures
  _config.ParseFile(config_filename);
  InitializeRoutingMap(_config);

  // Create NoC
  _booksim_noc.resize(SUBNETS);
  for (int noc_id = 0; noc_id < SUBNETS; noc_id++) {
    _booksim_noc[noc_id] = Network::New(_config, "noc_" + to_string(noc_id));
    _booksim_noc[noc_id]->DumpNodeMap();
  }

  string rf = _config.GetStr("routing_function") + "_" + _config.GetStr("topology");
  map<string, tRoutingFunction>::const_iterator rf_iter = gRoutingFunctionMap.find(rf);
  if (rf_iter == gRoutingFunctionMap.end()) {
    cerr << "Invalid routing function: " + rf << endl;
  }
  _routing_func = rf_iter->second;
  _lookahead_routing = !_config.GetInt("routing_delay");
  _wait_for_tail_credit = _config.GetInt("wait_for_tail_credit");

  _ejected_flits.resize(SUBNETS);
  for (int noc_id = 0; noc_id < SUBNETS; noc_id++) {
    _ejected_flits[noc_id] = new map<int, int>();
  }

  // Create buffer states for all routers in all subnets
  _buffer_state.resize(NOC_NODES);
  for (int node_id = 0; node_id < NOC_NODES; node_id++) {
    _buffer_state[node_id].resize(SUBNETS);
    for (int noc_id = 0; noc_id < SUBNETS; noc_id++) {
      std::ostringstream tmp_name;
      tmp_name << "terminal_buf_state_" << node_id << "_" << noc_id;
      auto* bs = new BufferState(_config, nullptr, tmp_name.str());
      int vc_alloc_delay = _config.GetInt("vc_alloc_delay");
      int sw_alloc_delay = _config.GetInt("sw_alloc_delay");
      int router_latency =
          _config.GetInt("routing_delay") +
          (_config.GetInt("speculative") ? max(vc_alloc_delay, sw_alloc_delay) : (vc_alloc_delay + sw_alloc_delay));
      int min_latency = 1 + _booksim_noc[noc_id]->GetInject(node_id)->GetLatency() + router_latency +
                        _booksim_noc[noc_id]->GetInjectCred(node_id)->GetLatency();
      bs->SetMinLatency(min_latency);
      _buffer_state[node_id][noc_id] = bs;
    }
  }

  // Create NoC AXI-streaming adapters
  _num_axis_master_endpoints = master_adapter_interface_types.size();
  noc_master_ports.init(_num_axis_master_endpoints);
  for (unsigned int port_id = 0; port_id < _num_axis_master_endpoints; port_id++)
    noc_master_ports[port_id].init(master_adapter_interface_types[port_id].size());
  _num_axis_slave_endpoints = slave_adapter_interface_types.size();
  noc_slave_ports.init(_num_axis_slave_endpoints);
  for (unsigned int port_id = 0; port_id < _num_axis_slave_endpoints; port_id++)
    noc_slave_ports[port_id].init(slave_adapter_interface_types[port_id].size());

  char adapter_name[25];
  std::string adapter_name_str;
  for (unsigned int slave_adapter_id = 0; slave_adapter_id < _num_axis_slave_endpoints; slave_adapter_id++) {
    adapter_name_str = "slave_adapter_" + std::to_string(slave_adapter_id);
    std::strcpy(adapter_name, adapter_name_str.c_str());
    axis_slave_adapter* s = new axis_slave_adapter(
        adapter_name, slave_adapter_node_ids[slave_adapter_id], 0, slave_adapter_interface_types[slave_adapter_id],
        slave_adapter_interface_dataw[slave_adapter_id], slave_adapter_node_periods[slave_adapter_id], &_config,
        _booksim_noc[0], _buffer_state[slave_adapter_node_ids[slave_adapter_id]][0], _routing_func, _lookahead_routing,
        _wait_for_tail_credit);
    for (unsigned int interface_id = 0; interface_id < slave_adapter_interface_types[slave_adapter_id].size();
         interface_id++)
      s->axis_interfaces[interface_id].ConnectToPort(noc_slave_ports[slave_adapter_id][interface_id]);
    s->noc_clk(noc_clk);
    s->node_clk(*node_clks[slave_adapter_node_clks[slave_adapter_id]]);
    s->adapter_clk(adapter_clk);
    s->rst(rst);
    _axis_slave_adapters.push_back(s);
    _axis_slave_adapters_node_id_to_idx[slave_adapter_node_ids[slave_adapter_id]] = slave_adapter_id;
  }

  for (unsigned int master_adapter_id = 0; master_adapter_id < _num_axis_master_endpoints; master_adapter_id++) {
    adapter_name_str = "master_adapter_" + std::to_string(master_adapter_id);
    std::strcpy(adapter_name, adapter_name_str.c_str());
    axis_master_adapter* m = new axis_master_adapter(
        adapter_name, master_adapter_node_ids[master_adapter_id], 0, master_adapter_interface_types[master_adapter_id],
        master_adapter_interface_dataw[master_adapter_id], &_config, _booksim_noc[0],
        _buffer_state[master_adapter_node_ids[master_adapter_id]][0], _routing_func, _lookahead_routing,
        _wait_for_tail_credit, _ejected_flits[0]);
    for (unsigned int interface_id = 0; interface_id < master_adapter_interface_types[master_adapter_id].size();
         interface_id++)
      m->axis_interfaces[interface_id].ConnectToPort(noc_master_ports[master_adapter_id][interface_id]);
    m->noc_clk(noc_clk);
    m->node_clk(*node_clks[master_adapter_node_clks[master_adapter_id]]);
    m->adapter_clk(adapter_clk);
    m->rst(rst);
    _axis_master_adapters.push_back(m);
    _axis_master_adapters_node_id_to_idx[master_adapter_node_ids[master_adapter_id]] = master_adapter_id;
  }

  SC_CTHREAD(Tick, noc_clk.pos());
  reset_signal_is(rst, true);
}

noc::~noc() {
  for (int noc_id = 0; noc_id < SUBNETS; noc_id++) {
    delete _booksim_noc[noc_id];
    delete _ejected_flits[noc_id];
    for (int node_id = 0; node_id < NOC_NODES; node_id++) {
      delete _buffer_state[node_id][noc_id];
    }
  }
}

Network* noc::GetNetwork(int subnet) {
  assert(subnet < SUBNETS);
  return _booksim_noc[subnet];
}

BookSimConfig* noc::GetConfig() { return &_config; }

int noc::GetNumNodes(int subnet) {
  assert(subnet < SUBNETS);
  return _booksim_noc[subnet]->NumNodes();
}

tRoutingFunction noc::GetRoutingFunction() const { return _routing_func; }

bool noc::IsLookaheadRouting() const { return _lookahead_routing; }

bool noc::IsWaitForTailCredit() const { return _wait_for_tail_credit; }

BufferState* noc::GetBufferState(int node, int subnet) {
  assert(node < NOC_NODES);
  assert(subnet < SUBNETS);
  return _buffer_state[node][subnet];
}

map<int, int>* noc::GetEjectedFlitsMap(int subnet) {
  assert(subnet < SUBNETS);
  return _ejected_flits[subnet];
}

void noc::Tick() {
  // Reset Code
  wait();

  while (true) {
    for (int noc_id = 0; noc_id < SUBNETS; noc_id++) {
      _booksim_noc[noc_id]->ReadInputs();
      for (int node_id = 0; node_id < NOC_NODES; node_id++) {
        map<int, int>::const_iterator iter = _ejected_flits[noc_id]->find(node_id);
        if (iter != _ejected_flits[noc_id]->end()) {
          Credit* const c = Credit::New();
          c->vc.insert(iter->second);
          _booksim_noc[noc_id]->WriteCredit(c, node_id);
        }
      }
      _ejected_flits[noc_id]->clear();
      _booksim_noc[noc_id]->Evaluate();
      _booksim_noc[noc_id]->WriteOutputs();
    }
    wait();
  }
}