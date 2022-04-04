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