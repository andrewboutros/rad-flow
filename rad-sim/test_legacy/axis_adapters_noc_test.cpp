#include <axis_adapters_noc_test.hpp>

RADSimConfig crossroads_config;
std::ofstream* radsim_log;
std::ostream* gWatchOut;

void axis_adapters_noc_test_system::ParseTestCase(const string& filename, int num_nodes) {
  std::ifstream infile(filename);

  int num_sources, num_sinks, num_transactions;
  if (!(infile >> num_sources >> num_sinks >> num_transactions)) {
    cerr << "Error parsing test case config file" << endl;
    exit(1);
  }

  std::vector<std::pair<int, std::vector<Flit::FlitType>>> source_adapters, sink_adapters;
  std::unordered_map<int, int> sources_node_id_to_idx, sinks_node_id_to_idx;

  *radsim_log << "Number of slave NoC adapters = " << num_sources << std::endl;
  *radsim_log << "Node \t#Interfaces \n";
  for (int i = 0; i < num_sources; i++) {
    int node_id, num_interfaces;
    if (!(infile >> node_id >> num_interfaces)) {
      cerr << "Error parsing test case config file" << endl;
      exit(1);
    }
    std::vector<Flit::FlitType> interface_types(num_interfaces, AXIS_TYPE0);
    source_adapters.push_back(std::make_pair(node_id, interface_types));
    *radsim_log << node_id << " \t" << num_interfaces << std::endl;
    sources_node_id_to_idx[node_id] = i;
  }
  *radsim_log << "Number of master NoC adapters = " << num_sinks << std::endl;
  for (int i = 0; i < num_sinks; i++) {
    int node_id, num_interfaces;
    if (!(infile >> node_id >> num_interfaces)) {
      cerr << "Error parsing test case config file" << endl;
      exit(1);
    }
    std::vector<Flit::FlitType> interface_types(num_interfaces, AXIS_TYPE0);
    sink_adapters.push_back(std::make_pair(node_id, interface_types));
    *radsim_log << node_id << " \t" << num_interfaces << std::endl;
    sinks_node_id_to_idx[node_id] = i;
  }
  _noc_wrapper = new noc("noc", _noc_config_filename, source_adapters, sink_adapters);
  _noc_wrapper->noc_clk(*noc_clk_signal);
  _noc_wrapper->node_clk(*node_clk_signal);
  _noc_wrapper->adapter_clk(*adapter_clk_signal);
  _noc_wrapper->rst(rst_signal);

  _test_driver =
      new axis_adapters_noc_test_driver("test_driver", num_nodes, sink_adapters, source_adapters, num_transactions);
  _test_driver->node_clk(*node_clk_signal);
  _test_driver->rst(rst_signal);
  _test_driver->SetIDTranslation(sinks_node_id_to_idx, sources_node_id_to_idx);

  master_axis_signals.resize(num_sinks);
  for (unsigned int sink_id = 0; sink_id < num_sinks; sink_id++)
    master_axis_signals[sink_id].resize(sink_adapters[sink_id].second.size());
  slave_axis_signals.resize(num_sources);
  for (unsigned int src_id = 0; src_id < num_sources; src_id++)
    slave_axis_signals[src_id].resize(source_adapters[src_id].second.size());

  for (unsigned int sink_id = 0; sink_id < num_sinks; sink_id++) {
    for (unsigned int sink_interface_id = 0; sink_interface_id < sink_adapters[sink_id].second.size();
         sink_interface_id++)
      master_axis_signals[sink_id][sink_interface_id].Connect(
          _noc_wrapper->noc_master_ports[sink_id][sink_interface_id],
          _test_driver->to_noc_master_interfaces[sink_id][sink_interface_id]);
  }

  for (unsigned int src_id = 0; src_id < num_sources; src_id++) {
    for (unsigned int src_interface_id = 0; src_interface_id < source_adapters[src_id].second.size();
         src_interface_id++)
      slave_axis_signals[src_id][src_interface_id].Connect(
          _test_driver->to_noc_slave_interfaces[src_id][src_interface_id],
          _noc_wrapper->noc_slave_ports[src_id][src_interface_id]);
  }

  for (int i = 0; i < num_transactions; i++) {
    int cycle, src, dst, global_id, axis_id;
    sc_bv<AXIS_USERW> axis_user;
    sc_bv<AXIS_DATAW> axis_data;
    if (!(infile >> cycle >> src >> dst >> global_id >> axis_data >> axis_id >> axis_user)) {
      cerr << "Error parsing test case config file" << endl;
      exit(1);
    }
    axis_transaction transaction;
    transaction.src_node = src;
    transaction.dest_node = dst;
    transaction.global_id = global_id;
    transaction.axis_data = axis_data;
    transaction.axis_id = axis_id;
    transaction.axis_user = axis_user;
    _test_driver->AddOutgoingTransactionToQueue(src, cycle, transaction);
  }
}

void axis_adapters_noc_test_system::GenerateRandomTestCase(const string& filename, int num_nodes, int num_transactions,
                                                           int num_sources, int num_sinks) {
  std::ofstream test_file(filename, std::ofstream::out);
  std::random_device r;
  std::default_random_engine e(r());

  // Write number of sources, sinks, and transaction to test configuration file
  test_file << num_sources << " " << num_sinks << " " << num_transactions << endl;

  // Randomly select source and sink nodes for this test
  std::vector<int> source_nodes, sink_nodes, source_nodes_interfaces, sink_nodes_interfaces;
  std::uniform_int_distribution<int> uniform_dist_nodes(0, num_nodes - 1);
  std::uniform_int_distribution<int> uniform_dist_num_interfaces(1, 10);
  int selected_node_id;
  for (unsigned int source_node_id = 0; source_node_id < num_sources;) {
    selected_node_id = uniform_dist_nodes(e);
    if (std::find(source_nodes.begin(), source_nodes.end(), selected_node_id) == source_nodes.end()) {
      source_nodes.push_back(selected_node_id);
      source_nodes_interfaces.push_back(uniform_dist_num_interfaces(e));
      source_node_id++;
    }
  }
  for (unsigned int sink_node_id = 0; sink_node_id < num_sinks;) {
    selected_node_id = uniform_dist_nodes(e);
    if (std::find(sink_nodes.begin(), sink_nodes.end(), selected_node_id) == sink_nodes.end()) {
      sink_nodes.push_back(selected_node_id);
      sink_nodes_interfaces.push_back(uniform_dist_num_interfaces(e));
      sink_node_id++;
    }
  }
  assert((source_nodes.size() == num_sources) && (sink_nodes.size() == num_sinks));

  // Write source/sink node IDs and number of interfaces per node to test configuration file

  for (unsigned int src_node_id = 0; src_node_id < source_nodes.size(); src_node_id++) {
    test_file << source_nodes[src_node_id] << " " << source_nodes_interfaces[src_node_id] << endl;
  }
  for (unsigned int sink_node_id = 0; sink_node_id < sink_nodes.size(); sink_node_id++) {
    test_file << sink_nodes[sink_node_id] << " " << sink_nodes_interfaces[sink_node_id] << endl;
  }

  // Generate random transaction contents for this test
  std::uniform_int_distribution<int> uniform_dist_source_idx(0, num_sources - 1);
  std::uniform_int_distribution<int> uniform_dist_sink_idx(0, num_sinks - 1);
  std::uniform_int_distribution<uint64_t> uniform_dist_data(0, UINT64_MAX);
  std::uniform_int_distribution<uint32_t> uniform_dist_user(0, UINT32_MAX);
  int src_idx, dst_idx;
  int transaction_ready_cycle = 5;
  for (int transaction_id = 0; transaction_id < num_transactions; transaction_id++) {
    src_idx = uniform_dist_source_idx(e);
    dst_idx = uniform_dist_sink_idx(e);
    test_file << transaction_ready_cycle << " " << source_nodes[src_idx] << " " << sink_nodes[dst_idx] << " "
              << transaction_id << " ";
    sc_bv<AXIS_DATAW> temp_data;
    for (int chunk_id = 0; chunk_id < AXIS_DATAW / 64; chunk_id++) {
      int start_idx = chunk_id * 64;
      int end_idx = std::min((chunk_id + 1) * 64, AXIS_DATAW);
      temp_data.range(end_idx - 1, start_idx) = uniform_dist_data(e);
    }
    sc_bv<AXIS_USERW> temp_user = uniform_dist_user(e);
    std::uniform_int_distribution<uint32_t> uniform_dist_id(0, sink_nodes_interfaces[dst_idx] - 1);
    test_file << temp_data << " " << uniform_dist_id(e) << " " << temp_user << endl;
  }
}

axis_adapters_noc_test_system::axis_adapters_noc_test_system(const sc_module_name& name, string& noc_config_filename,
                                                             string& test_config, int num_nodes)
    : sc_module(name) {
  // Create clocks
  node_clk_signal = new sc_clock("node_clk", FABRIC_PERIOD, SC_NS);
  noc_clk_signal = new sc_clock("noc_clk", NOC_PERIOD, SC_NS);
  adapter_clk_signal = new sc_clock("adapter_clk", ADAPTER_PERIOD, SC_NS);

  _noc_config_filename = noc_config_filename;

  GenerateRandomTestCase(test_config, num_nodes, 200, 3, 4);
  ParseTestCase(test_config, num_nodes);
}

axis_adapters_noc_test_system::~axis_adapters_noc_test_system() {
  delete _noc_wrapper;
  delete _test_driver;
  delete node_clk_signal;
  delete noc_clk_signal;
  delete adapter_clk_signal;
  sc_flit::FreeAllFlits();
}

axis_adapters_noc_test_driver::axis_adapters_noc_test_driver(
    const sc_module_name& name, int num_nodes,
    std::vector<std::pair<int, std::vector<Flit::FlitType>>> sink_adapters_info,
    std::vector<std::pair<int, std::vector<Flit::FlitType>>> src_adapters_info, int num_transactions)
    : sc_module(name) {
  _num_transactions = num_transactions;

  _outgoing_transactions.resize(num_nodes);
  _incoming_transactions.resize(num_nodes);
  to_noc_slave_interfaces.init(src_adapters_info.size());
  for (unsigned int src_interface_id = 0; src_interface_id < src_adapters_info.size(); src_interface_id++)
    to_noc_slave_interfaces[src_interface_id].init(src_adapters_info[src_interface_id].second.size());
  to_noc_master_interfaces.init(sink_adapters_info.size());
  for (unsigned int sink_interface_id = 0; sink_interface_id < sink_adapters_info.size(); sink_interface_id++)
    to_noc_master_interfaces[sink_interface_id].init(sink_adapters_info[sink_interface_id].second.size());

  SC_CTHREAD(sink, node_clk.pos());
  SC_CTHREAD(source, node_clk.pos());
}

axis_adapters_noc_test_driver::~axis_adapters_noc_test_driver(){};

void axis_adapters_noc_test_driver::AddOutgoingTransactionToQueue(int node_id, int cycle,
                                                                  const axis_transaction& transaction) {
  _outgoing_transactions[node_id].push(std::make_pair(cycle, transaction));
}

void axis_adapters_noc_test_driver::SetIDTranslation(std::unordered_map<int, int> node_id_to_master_idx,
                                                     std::unordered_map<int, int> node_id_to_slave_idx) {
  _node_id_to_master_interface_idx = std::move(node_id_to_master_idx);
  _node_id_to_slave_interface_idx = std::move(node_id_to_slave_idx);
}

void axis_adapters_noc_test_driver::source() {
  std::cout << "Running AXIS adapters test ... ";
  // Reset
  rst.write(true);
  for (auto& m : to_noc_master_interfaces) {
    for (unsigned int interface_id = 0; interface_id < m.size(); interface_id++) m[interface_id].Reset();
  }
  for (auto& s : to_noc_slave_interfaces) {
    for (unsigned int interface_id = 0; interface_id < s.size(); interface_id++) s[interface_id].Reset();
  }
  wait();
  rst.write(false);
  wait();

  std::random_device r;
  std::default_random_engine e(r());
  int transaction_id = 0;
  while (transaction_id < _num_transactions) {
    for (int node_id = 0; node_id < _outgoing_transactions.size(); node_id++) {
      if (_node_id_to_slave_interface_idx.find(node_id) == _node_id_to_slave_interface_idx.end()) continue;

      int src_idx = _node_id_to_slave_interface_idx[node_id];
      for (unsigned int interface_id = 0; interface_id < to_noc_slave_interfaces[src_idx].size(); interface_id++)
        to_noc_slave_interfaces[src_idx][interface_id].tvalid.write(false);

      if (!_outgoing_transactions[node_id].empty() &&
          _outgoing_transactions[node_id].front().first <= GetSimulationCycle(FABRIC_PERIOD)) {
        axis_transaction transaction = _outgoing_transactions[node_id].front().second;
        std::uniform_int_distribution<int> uniform_dist_num_interfaces(0, to_noc_slave_interfaces[src_idx].size() - 1);
        int src_interface = uniform_dist_num_interfaces(e);
        to_noc_slave_interfaces[src_idx][src_interface].tvalid.write(true);
        to_noc_slave_interfaces[src_idx][src_interface].tdest.write(transaction.dest_node);
        to_noc_slave_interfaces[src_idx][src_interface].tid.write(transaction.axis_id);
        to_noc_slave_interfaces[src_idx][src_interface].tdata.write(transaction.axis_data);
        to_noc_slave_interfaces[src_idx][src_interface].tuser.write(transaction.axis_user);
      }
    }
    wait();
    for (int node_id = 0; node_id < _outgoing_transactions.size(); node_id++) {
      if (_node_id_to_slave_interface_idx.find(node_id) == _node_id_to_slave_interface_idx.end()) continue;

      int src_idx = _node_id_to_slave_interface_idx[node_id];
      for (unsigned int src_interface = 0; src_interface < to_noc_slave_interfaces[src_idx].size(); src_interface++) {
        bool transaction_accepted = to_noc_slave_interfaces[src_idx][src_interface].tvalid.read() &&
                                    to_noc_slave_interfaces[src_idx][src_interface].tready.read();
        if (transaction_accepted) {
          _golden_output.push_back(_outgoing_transactions[node_id].front().second);
          _outgoing_transactions[node_id].pop();
          transaction_id++;
          /*std::cout << "Sent transaction " << transaction_id << " from node " << node_id << " interface "
                    << src_interface << " to node "
                    << to_noc_slave_interfaces[src_idx][src_interface].tdest.read().to_uint() << " interface "
                    << to_noc_slave_interfaces[src_idx][src_interface].tid.read().to_uint() << "!" << std::endl;*/
        }
      }
    }
  }

  for (auto& s : to_noc_slave_interfaces) {
    for (unsigned int interface_id = 0; interface_id < s.size(); interface_id++) s[interface_id].tvalid.write(false);
  }
  wait();
}

void axis_adapters_noc_test_driver::sink() {
  int received_transactions = 0;
  while (received_transactions < _num_transactions) {
    for (int node_id = 0; node_id < _incoming_transactions.size(); node_id++) {
      if (_node_id_to_master_interface_idx.find(node_id) == _node_id_to_master_interface_idx.end()) continue;

      int sink_idx = _node_id_to_master_interface_idx[node_id];
      for (unsigned int interface_id = 0; interface_id < to_noc_master_interfaces[sink_idx].size(); interface_id++) {
        if (to_noc_master_interfaces[sink_idx][interface_id].tvalid.read()) {
          axis_transaction transaction;
          transaction.dest_node = node_id;
          transaction.axis_id = to_noc_master_interfaces[sink_idx][interface_id].tid.read().to_uint();
          transaction.axis_user = to_noc_master_interfaces[sink_idx][interface_id].tuser.read();
          transaction.axis_data = to_noc_master_interfaces[sink_idx][interface_id].tdata.read();
          _incoming_transactions[node_id].push(std::make_pair(GetSimulationCycle(FABRIC_PERIOD), transaction));
          _output.push_back(transaction);
          received_transactions++;
          //std::cout << "Received " << received_transactions << " transaction(s)!" << std::endl;
        }
      }
    }
    wait();
  }

  sc_stop();

  // Write Output Files
  std::sort(_golden_output.begin(), _golden_output.end(), compare_axis_transactions());
  std::sort(_output.begin(), _output.end(), compare_axis_transactions());
  DumpOutputToFiles("/Users/andrew/PhD/dev/rad-sim/sim_output", "/Users/andrew/PhD/dev/rad-sim/golden_output");
  WriteReceivedTransactions("/Users/andrew/PhD/dev/rad-sim/output");

  // Write stats
  NoCTransactionTelemetry::DumpStatsToFile("/Users/andrew/PhD/dev/rad-sim/stats.csv");

  // Compare Outputs
  bool sim_successful = true;
  for (int i = 0; i < _output.size(); i++) {
    bool matching = (_output[i] == _golden_output[i]);
    sim_successful &= matching;
    /*if (!matching) {
      cout << "Got: " << _output[i] << endl;
      cout << "Exp: " << _golden_output[i] << endl;
      cout << "----------------" << endl;
    }*/
  }
  if (sim_successful) {
    cout << "PASS" << endl;
    delete radsim_log;
    exit(0);
  } else {
    cout << "FAIL" << endl;
    delete radsim_log;
    exit(1);
  }
}

void axis_adapters_noc_test_driver::DumpOutputToFiles(const std::string& output_filename,
                                                      const std::string& golden_output_filename) {
  std::ofstream gofile(golden_output_filename, std::ofstream::out);
  for (const auto& golden_output : _golden_output) {
    gofile << golden_output << endl;
  }
  gofile.close();

  std::ofstream ofile(output_filename, std::ofstream::out);
  for (const auto& output : _output) {
    ofile << output << endl;
  }
  ofile.close();
}

void axis_adapters_noc_test_driver::WriteReceivedTransactions(const std::string& filename) {
  std::ofstream ofile(filename, std::ofstream::out);
  for (int node_id = 0; node_id < _incoming_transactions.size(); node_id++) {
    while (!_incoming_transactions[node_id].empty()) {
      int cycle = _incoming_transactions[node_id].front().first;
      axis_transaction temp = _incoming_transactions[node_id].front().second;
      ofile << "[Node " << node_id << " @ " << cycle << "] ";
      ofile << temp << endl;
      _incoming_transactions[node_id].pop();
    }
  }
}

void ParseCrossroadsConfig() {
  RADSimConfig::AddIntKnob("noc_adapter_fifo_depth", 10);
  RADSimConfig::AddIntKnob("num_nodes", 32);
  RADSimConfig::AddDoubleKnob("fabric_period", 5.0);
  RADSimConfig::AddDoubleKnob("adapter_period", 1.25);
  RADSimConfig::AddDoubleKnob("noc_period", 1.0);
  RADSimConfig::AddIntKnob("num_virtual_channels", 5);
}

int sc_main(int argc, char* argv[]) {
  ParseCrossroadsConfig();
  string log_file = "/Users/andrew/PhD/dev/rad-sim/sim_log";
  string noc_config_file = "/Users/andrew/PhD/dev/rad-sim/src/noc_config";
  string test_config_file = "/Users/andrew/PhD/dev/rad-sim/test/test_adapters_config";

  radsim_log = new ofstream(log_file, std::ofstream::out);
  gWatchOut = radsim_log;

  axis_adapters_noc_test_system* top = new axis_adapters_noc_test_system("top", noc_config_file, test_config_file, 32);
  sc_start();

  return 0;
}