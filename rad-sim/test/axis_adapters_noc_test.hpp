#pragma once

#include <../src/noc/axis_interface.hpp>
#include <axis_master_adapter.hpp>
#include <axis_slave_adapter.hpp>
#include <cstdint>
#include <noc.hpp>
#include <queue>
#include <radsim_globals.hpp>
#include <radsim_utils.hpp>
#include <random>
#include <sstream>
#include <string>
#include <systemc>
#include <utility>
#include <vector>

struct axis_transaction {
  int src_node = 0, dest_node = 0, global_id = 0, axis_id = 0;
  sc_bv<AXIS_USERW> axis_user = "0x00";
  sc_bv<AXIS_DATAW> axis_data = "0x00";

  friend ostream& operator<<(ostream& os, const axis_transaction& transaction);
};

bool operator==(const axis_transaction& tr1, const axis_transaction& tr2) {
  return (tr1.axis_id == tr2.axis_id) && (tr1.axis_user == tr2.axis_user) && (tr1.axis_data == tr2.axis_data) &&
         (tr1.dest_node == tr2.dest_node);
}

ostream& operator<<(ostream& os, const axis_transaction& transaction) {
  os << transaction.dest_node << ", " << transaction.axis_id << ", " << transaction.axis_user << ", "
     << transaction.axis_data;
  return os;
}

struct compare_axis_transactions {
  inline bool operator()(const axis_transaction& tr1, const axis_transaction& tr2) {
    if (tr1.dest_node == tr2.dest_node) {
      if (tr1.axis_id == tr2.axis_id) {
        if (tr1.axis_user == tr2.axis_user) {
          return (tr1.axis_data.to_uint64() < tr2.axis_data.to_uint64());
        } else {
          return (tr1.axis_user.to_uint() < tr2.axis_user.to_uint());
        }
      } else {
        return (tr1.axis_id < tr2.axis_id);
      }
    } else {
      return (tr1.dest_node < tr2.dest_node);
    }
  }
};

class axis_adapters_noc_test_driver : public sc_module {
 private:
  std::vector<std::queue<std::pair<int, axis_transaction>>> _outgoing_transactions;
  std::vector<std::queue<std::pair<int, axis_transaction>>> _incoming_transactions;
  int _num_transactions;
  std::unordered_map<int, int> _node_id_to_master_interface_idx;
  std::unordered_map<int, int> _node_id_to_slave_interface_idx;

  std::vector<axis_transaction> _golden_output;
  std::vector<axis_transaction> _output;

 public:
  sc_in<bool> node_clk;
  sc_out<bool> rst;
  sc_vector<sc_vector<axis_master_port>> to_noc_slave_interfaces;
  sc_vector<sc_vector<axis_slave_port>> to_noc_master_interfaces;

  axis_adapters_noc_test_driver(const sc_module_name& name, int num_nodes,
                                std::vector<std::pair<int, std::vector<Flit::FlitType>>> sink_adapters_info,
                                std::vector<std::pair<int, std::vector<Flit::FlitType>>> src_adapters_info,
                                int num_transactions);
  ~axis_adapters_noc_test_driver();
  void SetIDTranslation(std::unordered_map<int, int> node_id_to_master_idx,
                        std::unordered_map<int, int> node_id_to_slave_idx);
  void AddOutgoingTransactionToQueue(int node_id, int cycle, const axis_transaction& transaction);
  void DumpOutputToFiles(const std::string& output_filename, const std::string& golden_output_filename);
  void WriteReceivedTransactions(const std::string& filename);
  void DumpStats(const std::string& filename);
  void source();
  void sink();

  SC_HAS_PROCESS(axis_adapters_noc_test_driver);
};

class axis_adapters_noc_test_system : public sc_module {
 private:
  noc* _noc_wrapper;
  axis_adapters_noc_test_driver* _test_driver;
  std::string _noc_config_filename;

 public:
  sc_signal<bool> rst_signal;
  std::vector<std::vector<axis_signal>> master_axis_signals, slave_axis_signals;
  sc_clock* node_clk_signal;
  sc_clock* noc_clk_signal;
  sc_clock* adapter_clk_signal;

  axis_adapters_noc_test_system(const sc_module_name& name, string& noc_config_filename, string& test_config,
                                int num_nodes);
  ~axis_adapters_noc_test_system();
  static void GenerateRandomTestCase(const string& filename, int num_nodes, int num_transactions, int min_masters,
                                     int min_slaves);
  void ParseTestCase(const string& filename, int num_nodes);
};