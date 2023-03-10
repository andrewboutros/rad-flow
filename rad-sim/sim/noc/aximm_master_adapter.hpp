#pragma once

#include <systemc.h>

#include <algorithm>
#include <aximm_interface.hpp>
#include <booksim_config.hpp>
#include <buffer_state.hpp>
#include <cmath>
#include <flit.hpp>
#include <network.hpp>
#include <queue>
#include <radsim_telemetry.hpp>
#include <radsim_utils.hpp>
#include <routefunc.hpp>
#include <sc_flit.hpp>

class aximm_master_adapter : public sc_module {
 private:
  unsigned int _node_id, _network_id;
  double _node_period, _adapter_period, _noc_period;
  unsigned int _interface_dataw;
  unsigned int _num_vcs, _ejection_afifo_depth, _output_afifo_depth;

  // Booksim-related variables
  BookSimConfig* _noc_config;
  Network* _noc;
  BufferState* _buffer_state;
  tRoutingFunction _routing_func;
  bool _lookahead_routing;
  bool _wait_for_tail_credit;
  map<int, int>* _ejected_flits;

  // Arbitration Output


 public:
  sc_in<bool> node_clk;
  sc_in<bool> adapter_clk;
  sc_in<bool> noc_clk;
  sc_in<bool> rst;
  sc_vector<aximm_master_port> axis_interfaces;

  aximm_master_adapter(const sc_module_name& name, int node_id, int network_id, 
                      unsigned int interface_dataw, BookSimConfig* noc_config, Network* noc, 
                      BufferState* buffer_state, tRoutingFunction routing_func, 
                      bool lookahead_routing, bool wait_for_tail_credit, 
                      map<int, int>* ejected_flits);
  ~aximm_master_adapter();

  void OutputEjection();
  void OutputDepacketization();
  void OutputInterface();
  SC_HAS_PROCESS(aximm_master_adapter);
};
