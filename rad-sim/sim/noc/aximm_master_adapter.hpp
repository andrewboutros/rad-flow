#pragma once

#include "aximm_interface.hpp"
#include "booksim_config.hpp"
#include "buffer_state.hpp"
#include "network.hpp"
#include "noc_utils.hpp"
#include "radsim_utils.hpp"
#include "routefunc.hpp"
#include "sc_flit.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <systemc.h>
#include <radsim_telemetry.hpp> //AKB added

class aximm_master_adapter : public sc_module {
private:
  // The ID and reconfigurable width of the node this adapter is connected to
  int _node_id;
  int _network_id;
  int _interface_dataw;
  int _axi_transaction_width;

  // Clock periods of the NoC, adapter, and node connected to it
  double _node_period;
  double _adapter_period;
  double _noc_period;

  // Pointers to Booksim datastructures: NoC, buffer states, routing info,
  // bookkeeping of ejected flits
  BookSimConfig *_noc_config;
  Network *_noc;
  BufferState *_buffer_state;
  tRoutingFunction _routing_func;
  bool _lookahead_routing;
  bool _wait_for_tail_credit;
  map<int, int> *_ejected_flits;

  // Request Interface variables
  Flit *_ejected_booksim_flit;
  unsigned int _ejection_afifo_depth;
  std::vector<std::queue<sc_flit>> _ejection_afifos;
  sc_vector<sc_signal<int>> _ejection_afifo_push_counter;
  sc_vector<sc_signal<int>> _ejection_afifo_pop_counter;
  std::queue<unsigned int> _ejection_afifo_priority;
  sc_signal<bool> _ejection_afifo_is_depacketizing;

  std::queue<sc_packet> _output_afifo;
  sc_packet _constructed_packet, _output_packet;
  bool _output_packet_ready;

  // Response Interface variables
  sc_signal<sc_uint<AXI4_IDW>> _i_id;
  sc_signal<sc_bv<AXI4_MAX_DATAW>> _i_payload;
  sc_signal<sc_bv<AXI4_CTRLW>> _i_ctrl;
  sc_signal<sc_bv<AXI4_USERW>> _i_user;
  sc_signal<sc_bv<AXI4_RESPW>> _i_resp;
  sc_signal<sc_bv<1>> _i_last;
  sc_signal<bool> _i_valid;
  sc_signal<sc_uint<3>> _i_type;
  sc_signal<sc_uint<NOC_LINKS_DEST_WIDTH>> _i_noc_dest;
  sc_signal<int> _i_unique_sim_id;

  bool _rready, _bready;

  unsigned int _injection_afifo_depth;
  std::queue<sc_flit> _injection_afifo;
  sc_signal<bool> _injection_afifo_full;
  sc_signal<sc_uint<1>> _injection_priority_setting;

  unsigned int _max_flits_per_transaction, _freq_ratio;
  sc_signal<uint8_t> _packetization_cycle;
  sc_signal<uint8_t> _num_packetization_flits;

  // unordered_map<int, int> _rid_to_src, _bid_to_src;
  bool _injection_flit_ready;
  sc_flit _to_be_injected_flit;
  int _last_vc_id;

  //AKB ADDED
  RADSimConfig* radsim_config;

public:
  sc_in<bool> node_clk;
  sc_in<bool> adapter_clk;
  sc_in<bool> noc_clk;
  sc_in<bool> rst;
  aximm_master_port aximm_interface;

  aximm_master_adapter(const sc_module_name &name, int node_id, int network_id,
                       BookSimConfig *noc_config, Network *noc,
                       BufferState *buffer_state, tRoutingFunction routing_func,
                       bool lookahead_routing, bool wait_for_tail_credit,
                       map<int, int> *ejected_flits,
                       unsigned int interface_dataw, double node_period,
                       double adapter_period,
                       RADSimConfig* radsim_config //AKB added
                       );
  ~aximm_master_adapter();

  void OutputInterface();
  void OutputDepacketization();
  void OutputEjection();

  void InputReady();
  void InputInterface();
  void InputPacketization();
  int GetInputDestinationNode(sc_bv<AXI4_ADDRW> &addr);
  int VCMapping(sc_flit &flit);
  int InverseVCMapping(int vc_id);
  void InputInjection();
  SC_HAS_PROCESS(aximm_master_adapter);
};