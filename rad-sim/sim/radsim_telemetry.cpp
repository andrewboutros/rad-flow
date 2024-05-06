#include <radsim_telemetry.hpp>

int NoCTransactionTelemetry::global_transaction_id = 0;
std::unordered_map<int, NoCTransactionTrace> NoCTransactionTelemetry::trace_bookkeeping;

NoCTransactionTelemetry::NoCTransactionTelemetry() {}
NoCTransactionTelemetry::~NoCTransactionTelemetry() {}

// Records the initiation timestamp of a transaction and returns its unique global ID
int NoCTransactionTelemetry::RecordTransactionInitiation(int src, int dest, int type, int dataw, int network_id) {
  NoCTransactionTrace entry;
  entry.src_node = src;
  entry.dest_node = dest;
  entry.transaction_type = type;
  entry.dataw = dataw;
  entry.network_id = network_id;
  entry.t_init = sc_time_stamp().to_double() / 1000;
  global_transaction_id++;
  trace_bookkeeping[global_transaction_id] = entry;
  return global_transaction_id;
}

// Records timestamp of finishing transaction packetization
void NoCTransactionTelemetry::RecordTransactionTailPacketization(int id) {
  double t = sc_time_stamp().to_double() / 1000;
  trace_bookkeeping[id].t_packet = t;
}

// Records timestamp of injecting head flit of a transaction into the NoC
void NoCTransactionTelemetry::RecordTransactionHeadInjection(int id) {
  trace_bookkeeping[id].t_inject = sc_time_stamp().to_double() / 1000;
}

// Records timestamp of ejecting tail flit of a transaction from the NoC
void NoCTransactionTelemetry::RecordTransactionTailEjection(int id) {
  trace_bookkeeping[id].t_eject = sc_time_stamp().to_double() / 1000;
}

// Records timestamp of finishing transaction de-packetization
void NoCTransactionTelemetry::RecordTransactionTailDepacketization(int id) {
  trace_bookkeeping[id].t_depacket = sc_time_stamp().to_double() / 1000;
}

// Records timestamp of receiving a transaction at the destination node
void NoCTransactionTelemetry::RecordTransactionReceipt(int id) {
  trace_bookkeeping[id].t_recieve = sc_time_stamp().to_double() / 1000;
  trace_bookkeeping[id].total_latency_ns = trace_bookkeeping[id].t_recieve - trace_bookkeeping[id].t_init;
}

// Sets the number of hops the flits of a transaction went through when traversing the NoC
void NoCTransactionTelemetry::UpdateHops(int id, int num_hops) { trace_bookkeeping[id].num_hops = num_hops; }

void NoCTransactionTelemetry::DumpStatsToFile(const std::string& filename) {
  std::ofstream ofile(filename, std::ofstream::out);
  ofile << "id, noc_id, src, dest, dataw, hops, t_init, t_packet, t_inject, t_eject, t_depacket, t_receive, latency" << endl;
  for (auto& entry : trace_bookkeeping) {
    ofile << entry.first << ", " << entry.second.network_id << ", " << entry.second.src_node << ", " 
          << entry.second.dest_node << ", " << entry.second.dataw << ", " << entry.second.num_hops << ", " 
          << entry.second.t_init << ", " << entry.second.t_packet << ", " << entry.second.t_inject << ", " 
          << entry.second.t_eject << ", " << entry.second.t_depacket << ", " << entry.second.t_recieve 
          << ", " << entry.second.total_latency_ns << endl;
  }
  ofile.close();
}

std::vector<double> NoCTransactionTelemetry::DumpTrafficFlows(const std::string& filename, unsigned int cycle_count, 
  std::vector<std::vector<std::set<std::string>>>& node_module_names) {
  double sim_driver_period = radsim_config.GetDoubleKnob("sim_driver_period") / 1000000000.0;
  unsigned int num_nocs = radsim_config.GetIntKnob("noc_num_nocs");
  std::vector<std::vector<std::unordered_map<unsigned int, unsigned int>>> traffic_bits(num_nocs);
  std::vector<std::vector<std::unordered_map<unsigned int, unsigned int>>> traffic_num_hops(num_nocs);
  for (unsigned int noc_id = 0; noc_id < num_nocs; noc_id++) {
    unsigned int num_nodes = radsim_config.GetIntVectorKnob("noc_num_nodes", noc_id);
    traffic_bits[noc_id].resize(num_nodes);
    traffic_num_hops[noc_id].resize(num_nodes);
  }
  for (auto& entry : trace_bookkeeping) {
    unsigned int noc_id = (unsigned int) entry.second.network_id;
    unsigned int src = (unsigned int) entry.second.src_node;
    unsigned int dest = (unsigned int) entry.second.dest_node;
    if (traffic_bits[noc_id][src].find(dest) == traffic_bits[noc_id][src].end())
      traffic_bits[noc_id][src][dest] = entry.second.dataw;
    else
      traffic_bits[noc_id][src][dest] += entry.second.dataw;
    traffic_num_hops[noc_id][src][dest] = entry.second.num_hops - 1;
  }
  
  std::vector<double> aggregate_bandwidths;
  for (unsigned int noc_id = 0; noc_id < num_nocs; noc_id++) {
    double aggregate_bandwidth = 0.0;
    std::ofstream traffic_file(filename + "_noc" + std::to_string(noc_id) + ".xml", std::ofstream::out);
    traffic_file << "<traffic_flows>" << endl;
    unsigned int num_nodes = radsim_config.GetIntVectorKnob("noc_num_nodes", noc_id);
    for (unsigned int src_id = 0; src_id < num_nodes; src_id++) {
      if (traffic_bits[noc_id][src_id].size() > 0) {
        for (auto& flow : traffic_bits[noc_id][src_id]) {
          traffic_file << "\t<single_flow src=\".*";
          std::string src_name = *node_module_names[noc_id][src_id].begin();
          std::string dst_name = *node_module_names[noc_id][flow.first].begin();
          traffic_file << "noc_router_" << src_name << ".*\" dst=\".*";
          traffic_file << "noc_router_" << dst_name << ".*\" bandwidth=\"";
          double bandwidth = flow.second / (cycle_count * sim_driver_period);
          aggregate_bandwidth += (traffic_num_hops[noc_id][src_id][flow.first] * bandwidth);
          traffic_file << bandwidth << "\"/>" << endl;
        }
      }
    }
    traffic_file << "</traffic_flows>" << endl;
    aggregate_bandwidths.push_back(aggregate_bandwidth);
  }
  return aggregate_bandwidths;
}

std::stringstream NoCFlitTelemetry::ostream;

NoCFlitTelemetry::NoCFlitTelemetry() {}
NoCFlitTelemetry::~NoCFlitTelemetry() {}

void NoCFlitTelemetry::DumpNoCFlitTracesToFile(const std::string& filename) {
  std::ofstream ofile(filename, std::ofstream::out);
  ofile << "t_trace, link_src_router, link_dest_router, flit_id, flit_type, is_channel_output" << endl;
  ofile << NoCFlitTelemetry::ostream.rdbuf();
  ofile.close();
}

SimLog::SimLog() {
  verbosity = 0;
}

SimLog::SimLog(unsigned int verbosity_level, std::string log_filename) {
  verbosity = verbosity_level;
  log_file.open(log_filename);
}

SimLog::~SimLog() { log_file.close(); }

void SimLog::SetLogSettings(unsigned int verbosity_level, std::string log_filename) {
  verbosity = verbosity_level;
  log_file.open(log_filename);
}

void SimLog::log(debug_t, std::string msg, sc_module_name module, bool log_to_file) {
  if (verbosity >= 4) {
    std::cout << "\033[32m[DEBUG] " << module << " " << msg << "\033[0m" << std::endl;
    if (log_to_file) {
      log_file << "\033[32m[DEBUG] " << module << " " << msg << "\033[0m" << std::endl;
    }
  }
}

void SimLog::log(info_t, std::string msg, sc_module_name module, bool log_to_file) {
  if (verbosity >= 3) {
    std::cout << "[INFO] " << module << " " << msg << std::endl;
    if (log_to_file) {
      log_file << "[INFO] " << module << " " << msg << std::endl;
    }
  }
}

void SimLog::log(trace_t, std::string msg, sc_module_name module, bool log_to_file) {
  if (verbosity >= 2) {
    std::cout << "\033[36m[TRACE] " << module << " @ " << GetSimulationCycle() << ": " << msg << "\033[0m"
              << std::endl;
    if (log_to_file) {
      log_file << "[TRACE] " << module << " @ " << GetSimulationCycle() << ": " << msg << std::endl;
    }
  }
}

void SimLog::log(warning_t, std::string msg, sc_module_name module, bool log_to_file) {
  if (verbosity >= 1) {
    std::cout << "\033[1;33m[WARNING] " << module << " " << msg << "\033[0m" << std::endl;
    if (log_to_file) {
      log_file << "[WARNING] " << module << " " << msg << std::endl;
    }
  }
}

void SimLog::log(critical_t, std::string msg, sc_module_name module, bool log_to_file) {
  std::cout << "\033[1m[INFO] " << module << " " << msg << "\033[0m" << std::endl;
  if (log_to_file) {
    log_file << "[INFO] " << module << " " << msg << std::endl;
  }
}

void SimLog::log(fatal_t, std::string msg, sc_module_name module, bool log_to_file) {
  std::cout << "\033[1;31m[ERROR] " << module << " " << msg << "\033[0m" << std::endl;
  if (log_to_file) {
    log_file << "[ERROR] " << module << " " << msg << std::endl;
  }
  exit(1);
}

SimTraceRecording::SimTraceRecording() {
  num_traces_monitored = 0;
}

SimTraceRecording::SimTraceRecording(std::string filename, unsigned int num_traces) {
  num_traces_monitored = num_traces;
  trace_filename = filename;
  trace_cycles.resize(num_traces_monitored);
}

SimTraceRecording::~SimTraceRecording() {}

void SimTraceRecording::SetTraceRecordingSettings(std::string filename, unsigned int num_traces) {
  num_traces_monitored = num_traces;
  trace_filename = filename;
  trace_cycles.resize(num_traces_monitored);
}

void SimTraceRecording::record_event(unsigned int trace_id) {
  trace_cycles[trace_id].push_back(GetSimulationCycle());
}

void SimTraceRecording::dump_traces() {
  std::ofstream trace_file;
  trace_file.open(trace_filename);
  for (unsigned int trace_id = 0; trace_id < num_traces_monitored; trace_id++) {
    for (unsigned int i = 0; i < trace_cycles[trace_id].size(); i++)
      trace_file << trace_cycles[trace_id][i] << " ";
    trace_file << "\n";
  }
}