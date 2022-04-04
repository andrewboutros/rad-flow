#pragma once

#include <systemc.h>

#include <iostream>
#include <radsim_globals.hpp>
#include <string>
#include <unordered_map>

// Struct capsulating information for a transaction traversing through the NoC
struct NoCTransactionTrace {
  int src_node;             // Source node initiating the transaction
  int dest_node;            // Destination node receiving the transaction
  int num_hops;             // Number of hops between source and destination nodes
  int transaction_type;     // Transaction type
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
  static int RecordTransactionInitiation(int src, int dest, int type);
  static void RecordTransactionTailPacketization(int id);
  static void RecordTransactionHeadInjection(int id);
  static void RecordTransactionTailEjection(int id);
  static void RecordTransactionTailDepacketization(int id);
  static void RecordTransactionReceipt(int id);
  static void UpdateHops(int id, int num_hops);
  static void DumpStatsToFile(const std::string& filename);
};
