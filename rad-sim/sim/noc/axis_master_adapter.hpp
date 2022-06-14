#pragma once

#include <systemc.h>

#include <algorithm>
#include <axis_interface.hpp>
#include <booksim_config.hpp>
#include <buffer_state.hpp>
#include <cmath>
#include <network.hpp>
#include <queue>
#include <radsim_telemetry.hpp>
#include <radsim_utils.hpp>
#include <routefunc.hpp>
#include <sc_flit.hpp>

class axis_master_adapter : public sc_module {
 private:
  unsigned int _node_id;
  unsigned int _network_id;
  unsigned int _num_axis_interfaces;
  std::vector<int> _num_flits;
  unsigned int _max_num_flits;
  unsigned int _num_vcs;
  std::vector<unsigned int> _interface_dataw;

  BookSimConfig* _noc_config;
  Network* _noc;
  BufferState* _buffer_state;
  tRoutingFunction _routing_func;
  bool _lookahead_routing;
  bool _wait_for_tail_credit;
  map<int, int>* _ejected_flits;

  Flit* _ejected_booksim_flit;
  unsigned int _ejection_afifo_depth;
  std::vector<std::queue<sc_flit>> _ejection_afifos;
  sc_vector<sc_signal<int>> _ejection_afifo_push_counter;
  sc_vector<sc_signal<int>> _ejection_afifo_pop_counter;
  std::queue<unsigned int> _ejection_afifo_priority;
  sc_signal<bool> _ejection_afifo_is_depacketizing;
  sc_signal<int> _insert_credit_vc;

  std::vector<std::queue<sc_packet>> _output_afifos;
  unsigned int _output_afifo_depth;
  sc_packet _constructed_packet;
  std::vector<bool> _output_packet_ready;
  std::vector<int> _output_chunk;

 public:
  sc_in<bool> node_clk;
  sc_in<bool> adapter_clk;
  sc_in<bool> noc_clk;
  sc_in<bool> rst;
  sc_vector<axis_master_port> axis_interfaces;

  axis_master_adapter(const sc_module_name& name, int node_id, int network_id,
                      std::vector<Flit::FlitType>& interface_types, std::vector<unsigned int>& interface_dataw,
                      BookSimConfig* noc_config, Network* noc, BufferState* buffer_state, tRoutingFunction routing_func,
                      bool lookahead_routing, bool wait_for_tail_credit, map<int, int>* ejected_flits);
  ~axis_master_adapter();

  void write_sc_packet_to_axis_output(sc_packet& packet, axis_master_port& axis_port, int output_chunk,
                                      unsigned int interface_id);
  void OutputInterface();
  void OutputDepacketization();
  void OutputEjection();
  SC_HAS_PROCESS(axis_master_adapter);
};
