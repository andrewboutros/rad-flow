#pragma once

#include <systemc.h>
#include <radsim_utils.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <set>

// Struct capsulating information for a transaction traversing through the NoC
struct NoCTransactionTrace {
  int src_node;             // Source node initiating the transaction
  int dest_node;            // Destination node receiving the transaction
  int num_hops;             // Number of hops between source and destination nodes
  int transaction_type;     // Transaction type
  int dataw;                // Data width of the transaction in bits
  int network_id;           // NoC ID of the transaction
  double t_init;            // Time at which transaction is initiated
  double t_packet;          // Time at which transaction is packetized
  double t_inject;          // Time at which head flit of transaction is injected into the NoC
  double t_eject;           // Time at which tail flit of transaction is ejected from the NoC
  double t_depacket;        // Time at which transaction is depacketized
  double t_recieve;         // Time at which transaction is received
  double total_latency_ns;  // Total latency (in ns) between initiating & receiving transaction

  NoCTransactionTrace()
      : src_node(-1),
        dest_node(-1),
        num_hops(0),
        transaction_type(0),
        dataw(0),
        network_id(0),
        t_init(0.0),
        t_packet(0.0),
        t_inject(0.0),
        t_eject(0.0),
        t_depacket(0.0),
        t_recieve(0.0),
        total_latency_ns(0.0) {}
};

// Class for recording and storing transaction events and statistics
class NoCTransactionTelemetry {
 public:
  static int global_transaction_id;  // Unique integer ID for each transaction traversing the NoC
  static std::unordered_map<int, NoCTransactionTrace> trace_bookkeeping;  // Hashtable for storing transaction traces

  NoCTransactionTelemetry();
  ~NoCTransactionTelemetry();
  static int RecordTransactionInitiation(int src, int dest, int type, int dataw, int network_id);
  static void RecordTransactionTailPacketization(int id);
  static void RecordTransactionHeadInjection(int id);
  static void RecordTransactionTailEjection(int id);
  static void RecordTransactionTailDepacketization(int id);
  static void RecordTransactionReceipt(int id);
  static void UpdateHops(int id, int num_hops);
  static void DumpStatsToFile(const std::string& filename);
  static std::vector<double> DumpTrafficFlows(const std::string& filename, unsigned int cycle_count,
    std::vector<std::vector<std::set<std::string>>>& node_module_names);
};

// Class for recording and storing flit traces
class NoCFlitTelemetry {
 public:
  static std::stringstream ostream;

  NoCFlitTelemetry();
  ~NoCFlitTelemetry();
  static void DumpNoCFlitTracesToFile(const std::string& filename);
};

struct debug_t {};
constexpr auto debug = debug_t{};
struct trace_t {};
constexpr auto sim_trace = trace_t{};
struct info_t {};
constexpr auto info = info_t{};
struct critical_t {};
constexpr auto critical = critical_t{};
struct warning_t {};
constexpr auto warning = warning_t{};
struct fatal_t {};
constexpr auto error = fatal_t{};

class SimLog {
 private:
  unsigned int verbosity;
  std::ofstream log_file;

 public:
  SimLog();
  SimLog(unsigned int verbosity_level, std::string log_filename);
  ~SimLog();
  void SetLogSettings(unsigned int verbosity_level, std::string log_filename);
  void log(debug_t, std::string msg, sc_module_name module = "", bool log_to_file = true);
  void log(trace_t, std::string msg, sc_module_name module, bool log_to_file = true);
  void log(info_t, std::string msg, sc_module_name module = "", bool log_to_file = true);
  void log(critical_t, std::string msg, sc_module_name module = "", bool log_to_file = true);
  void log(warning_t, std::string msg, sc_module_name module = "", bool log_to_file = true);
  void log(fatal_t, std::string msg, sc_module_name module = "", bool log_to_file = true);
};

class SimTraceRecording {
 private:
  std::vector<std::vector<unsigned int>> trace_cycles;
  unsigned int num_traces_monitored;
  std::string trace_filename;

 public:
  SimTraceRecording();
  SimTraceRecording(std::string filename, unsigned int num_traces);
  ~SimTraceRecording();
  void SetTraceRecordingSettings(std::string filename, unsigned int num_traces);
  void record_event(unsigned int trace_id);
  void dump_traces();
};

extern SimLog sim_log;
extern SimTraceRecording sim_trace_probe;
extern std::ostream* gWatchOut;