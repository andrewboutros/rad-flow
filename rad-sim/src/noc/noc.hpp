#pragma once

#include <systemc.h>

#include <axis_interface.hpp>
#include <axis_master_adapter.hpp>
#include <axis_slave_adapter.hpp>
#include <booksim_config.hpp>
#include <buffer_state.hpp>
#include <network.hpp>
#include <radsim_globals.hpp>
#include <routefunc.hpp>
#include <sstream>

// NoC SystemC wrapper around all Booksim-related datastructures
class noc : public sc_module {
 private:
  BookSimConfig _config;                                 // Booksim NoC configuration
  std::vector<Network*> _booksim_noc;                    // Pointers to all Booksim sub-networks
  tRoutingFunction _routing_func;                        // Booksim NoC routing function
  bool _lookahead_routing;                               // Is lookahead routing used?
  bool _wait_for_tail_credit;                            // Do we wait for tail credit?
  std::vector<std::vector<BufferState*>> _buffer_state;  // Vector of buffer state pointers for all sub-networks
  std::vector<std::map<int, int>*> _ejected_flits;       // Ejected flit map <node, virtual channel> of all sub-networks

  int _num_axis_slave_endpoints, _num_axis_master_endpoints;
  std::vector<axis_master_adapter*> _axis_master_adapters;
  std::vector<axis_slave_adapter*> _axis_slave_adapters;
  std::unordered_map<int, int> _axis_master_adapters_node_id_to_idx;
  std::unordered_map<int, int> _axis_slave_adapters_node_id_to_idx;
  std::unordered_map<int, int> _module_noc_assignment;

 public:
  sc_in<bool> adapter_clk;
  sc_in<bool> noc_clk;
  sc_in<bool> rst;
  sc_vector<sc_vector<axis_master_port>> noc_master_ports;
  sc_vector<sc_vector<axis_slave_port>> noc_slave_ports;

  noc(const sc_module_name& name, string& config_filename, std::vector<sc_clock*>& node_clks,
      std::vector<int>& slave_adapter_node_ids, std::vector<std::vector<Flit::FlitType>>& slave_adapter_interface_types,
      std::vector<float>& slave_adapter_node_periods, std::vector<unsigned int>& slave_adapter_node_clks,
      std::vector<std::vector<unsigned int>>& slave_adapter_interface_dataw, std::vector<int>& master_adapter_node_ids,
      std::vector<std::vector<Flit::FlitType>>& master_adapter_interface_types,
      std::vector<std::vector<unsigned int>>& master_adapter_interface_dataw,
      std::vector<unsigned int>& master_adapter_node_clks);
  ~noc();

  Network* GetNetwork(int subnet);
  BookSimConfig* GetConfig();
  int GetNumNodes(int subnet);
  tRoutingFunction GetRoutingFunction() const;
  bool IsLookaheadRouting() const;
  bool IsWaitForTailCredit() const;
  BufferState* GetBufferState(int node, int subnet);
  map<int, int>* GetEjectedFlitsMap(int subnet);

  void Tick();
  SC_HAS_PROCESS(noc);
};
