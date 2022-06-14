#include <radsim_telemetry.hpp>

int NoCTransactionTelemetry::global_transaction_id = 0;
std::unordered_map<int, NoCTransactionTrace> NoCTransactionTelemetry::trace_bookkeeping;

NoCTransactionTelemetry::NoCTransactionTelemetry() {}
NoCTransactionTelemetry::~NoCTransactionTelemetry() {}

// Records the initiation timestamp of a transaction and returns its unique global ID
int NoCTransactionTelemetry::RecordTransactionInitiation(int src, int dest, int type) {
  NoCTransactionTrace entry;
  entry.src_node = src;
  entry.dest_node = dest;
  entry.transaction_type = type;
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
  ofile << "id, hops, t_init, t_packet, t_inject, t_eject, t_depacket, t_receive, latency" << endl;
  for (auto& entry : trace_bookkeeping) {
    ofile << entry.first << ", " << entry.second.num_hops << ", " << entry.second.t_init << ", "
          << entry.second.t_packet << ", " << entry.second.t_inject << ", " << entry.second.t_eject << ", "
          << entry.second.t_depacket << ", " << entry.second.t_recieve << ", " << entry.second.total_latency_ns << endl;
  }
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
  num_modules_monitored = 0;
}

SimTraceRecording::SimTraceRecording(std::string filename, unsigned int num_traces, unsigned int num_modules) {
  num_traces_monitored = num_traces;
  num_modules_monitored = num_modules;
  trace_filename = filename;
  trace_cycles.resize(num_traces_monitored);
  for (unsigned int trace_id = 0; trace_id < num_traces_monitored; trace_id++)
    trace_cycles[trace_id].resize(num_modules_monitored);
}

SimTraceRecording::~SimTraceRecording() {}

void SimTraceRecording::SetTraceRecordingSettings(std::string filename, unsigned int num_traces, unsigned int num_modules) {
  num_traces_monitored = num_traces;
  num_modules_monitored = num_modules;
  trace_filename = filename;
  trace_cycles.resize(num_traces_monitored);
  for (unsigned int trace_id = 0; trace_id < num_traces_monitored; trace_id++)
    trace_cycles[trace_id].resize(num_modules_monitored);
}

void SimTraceRecording::record_event(unsigned int module_id, unsigned int trace_id) {
  trace_cycles[trace_id][module_id].push_back(GetSimulationCycle());
}

void SimTraceRecording::dump_traces() {
  std::ofstream trace_file;
  trace_file.open(trace_filename);
  for (unsigned int module_id = 0; module_id < num_modules_monitored; module_id++) {
    for (unsigned int trace_id = 0; trace_id < num_traces_monitored; trace_id++) {
      for (unsigned int i = 0; i < trace_cycles[trace_id][module_id].size(); i++)
        trace_file << trace_cycles[trace_id][module_id][i] << " ";
      trace_file << "\n";
    }
  }
}