#pragma once

#include <systemc.h>

#include <axis_interface.hpp>
#include <axis_master_adapter.hpp>
#include <axis_slave_adapter.hpp>
#include <booksim_config.hpp>
#include <buffer_state.hpp>
#include <network.hpp>
#include <radsim_utils.hpp>
#include <routefunc.hpp>
#include <sstream>

// NoC SystemC wrapper around all Booksim-related datastructures
class noc_axis : public sc_module {
 private:
  int _noc_id;
  int _num_noc_nodes;
  BookSimConfig _config;                                 // Booksim NoC configuration
  Network* _booksim_noc;                                 // Pointers to Booksim network
  tRoutingFunction _routing_func;                        // Booksim NoC routing function
  bool _lookahead_routing;                               // Is lookahead routing used?
  bool _wait_for_tail_credit;                            // Do we wait for tail credit?
  std::vector<BufferState*> _buffer_state;               // Vector of buffer state pointers
  std::map<int, int>* _ejected_flits;                    // Ejected flit map <node, virtual channel>

  unsigned int _num_axis_slave_endpoints, _num_axis_master_endpoints;
  std::vector<axis_master_adapter*> _axis_master_adapters;
  std::vector<axis_slave_adapter*> _axis_slave_adapters;

 public:
  sc_in<bool> noc_clk;
  sc_in<bool> rst;
  sc_vector<sc_vector<axis_master_port>> noc_master_ports;
  sc_vector<sc_vector<axis_slave_port>> noc_slave_ports;

  noc_axis(const sc_module_name& name, 
           int noc_id, 
           std::vector<sc_clock*>& adapter_clks, 
           std::vector<sc_clock*>& module_clks,
           std::vector<AdapterInfo>& master_adapter_info,
           std::vector<AdapterInfo>& slave_adapter_info);
  ~noc_axis();

  Network* GetNetwork();
  BookSimConfig* GetConfig();
  int GetNumNodes();
  tRoutingFunction GetRoutingFunction() const;
  bool IsLookaheadRouting() const;
  bool IsWaitForTailCredit() const;
  BufferState* GetBufferState(int node);
  map<int, int>* GetEjectedFlitsMap();

  void Tick();
  SC_HAS_PROCESS(noc_axis);
};
