#pragma once

#include <systemc.h>

#include <aximm_interface.hpp>
#include <aximm_master_adapter.hpp>
#include <aximm_slave_adapter.hpp>
#include <axis_interface.hpp>
#include <axis_master_adapter.hpp>
#include <axis_slave_adapter.hpp>
#include <booksim_config.hpp>
#include <buffer_state.hpp>
#include <network.hpp>
#include <radsim_utils.hpp>
#include <routefunc.hpp>
#include <sstream>

//#include <design_context.hpp> //AKB ADDED
class RADSimDesignContext; //AKB ADDED

// NoC SystemC wrapper around all Booksim-related datastructures
class radsim_noc : public sc_module {
private:
  int _rad_id;
  std::string _portal_slave_name;
  int _noc_id;
  int _num_noc_nodes;
  BookSimConfig _config;                    // Booksim NoC configuration
  Network *_booksim_noc;                    // Pointers to Booksim network
  tRoutingFunction _routing_func;           // Booksim NoC routing function
  bool _lookahead_routing;                  // Is lookahead routing used?
  bool _wait_for_tail_credit;               // Do we wait for tail credit?
  std::vector<BufferState *> _buffer_state; // Vector of buffer state pointers
  std::map<int, int>
      *_ejected_flits; // Ejected flit map <node, virtual channel>

  unsigned int _num_axis_slave_endpoints, _num_axis_master_endpoints;
  unsigned int _num_aximm_slave_endpoints, _num_aximm_master_endpoints;
  std::vector<axis_master_adapter *> _axis_master_adapters;
  std::vector<axis_slave_adapter *> _axis_slave_adapters;
  std::vector<aximm_master_adapter *> _aximm_master_adapters;
  std::vector<aximm_slave_adapter *> _aximm_slave_adapters;

public:
  sc_in<bool> noc_clk;
  sc_in<bool> rst;
  sc_vector<sc_vector<axis_master_port>> noc_axis_master_ports;
  sc_vector<sc_vector<axis_slave_port>> noc_axis_slave_ports;
  sc_vector<aximm_master_port> noc_aximm_master_ports;
  sc_vector<aximm_slave_port> noc_aximm_slave_ports;

  radsim_noc(const sc_module_name &name, unsigned int rad_id, std::string portal_slave_name, int noc_id,
             std::vector<sc_clock *> &adapter_clks,
             std::vector<sc_clock *> &module_clks,
             std::vector<AdapterInfo> &axis_master_adapter_info,
             std::vector<AdapterInfo> &axis_slave_adapter_info,
             std::vector<AdapterInfo> &aximm_master_adapter_info,
             std::vector<AdapterInfo> &aximm_slave_adapter_info, 
             RADSimDesignContext* radsim_design); //AKB: added last argument
  ~radsim_noc();

  Network *GetNetwork();
  BookSimConfig *GetConfig();
  int GetNumNodes();
  tRoutingFunction GetRoutingFunction() const;
  bool IsLookaheadRouting() const;
  bool IsWaitForTailCredit() const;
  BufferState *GetBufferState(int node);
  map<int, int> *GetEjectedFlitsMap();

  void Tick();
  SC_HAS_PROCESS(radsim_noc);
};
