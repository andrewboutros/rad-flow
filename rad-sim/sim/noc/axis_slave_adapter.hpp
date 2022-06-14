#pragma once

#include <systemc.h>

#include <algorithm>
#include <axis_interface.hpp>
#include <booksim_config.hpp>
#include <buffer_state.hpp>
#include <cmath>
#include <flit.hpp>
#include <network.hpp>
#include <queue>
#include <radsim_telemetry.hpp>
#include <noc_utils.hpp>
#include <routefunc.hpp>
#include <sc_flit.hpp>

class axis_slave_adapter : public sc_module {
 private:
  unsigned int _node_id;  // Node ID of this adapter
  double _node_period, _adapter_period, _noc_period;
  unsigned int _network_id;
  unsigned int _num_axis_interfaces;
  std::vector<Flit::FlitType> _interface_types;
  std::vector<unsigned int> _interface_dataw;
  std::vector<unsigned int> _num_flits_per_packet;
  unsigned int _max_num_flits_per_packet;
  unsigned int _freq_ratio;

  BookSimConfig* _noc_config;
  Network* _noc;
  BufferState* _buffer_state;
  tRoutingFunction _routing_func;
  bool _lookahead_routing;
  bool _wait_for_tail_credit;

  // Input arbitration variables
  std::queue<unsigned int> _axis_interface_priority;
  std::vector<bool> _tready_values;
  sc_signal<bool> _injection_afifo_full;
  sc_signal<bool> _packetization_busy;

  // Input interfacing variables
  axis_signal _input_axis_transaction;
  sc_signal<unsigned int> _input_axis_transaction_id;
  sc_signal<Flit::FlitType> _input_axis_transaction_type;
  sc_signal<unsigned int> _input_axis_transaction_interface;

  // Input packetization variables
  std::queue<sc_flit> _injection_afifo;
  unsigned int _injection_afifo_depth;
  int _packetization_cycle;

  // Input injection variables
  bool _injection_flit_ready;
  sc_flit _to_be_injected_flit;

 public:
  sc_in<bool> node_clk;
  sc_in<bool> adapter_clk;
  sc_in<bool> noc_clk;
  sc_in<bool> rst;
  sc_vector<axis_slave_port> axis_interfaces;

  axis_slave_adapter(const sc_module_name& name, int node_id, int network_id,
                     std::vector<Flit::FlitType>& interface_types, std::vector<unsigned int>& interface_dataw,
                     double node_period, double adapter_period, BookSimConfig* noc_config, Network* noc, 
                     BufferState* buffer_state, tRoutingFunction routing_func, bool lookahead_routing, 
                     bool wait_for_tail_credit);
  ~axis_slave_adapter();

  int GetInputDestinationNode(const sc_bv<AXIS_DESTW> axis_transaction_dest);
  void InputReady();
  void InputInterface();
  void InputPacketization();
  void InputInjection();
  SC_HAS_PROCESS(axis_slave_adapter);
};