#pragma once

#include <systemc.h>

#include <flit.hpp>
#include <iomanip>
#include <ostream>
#include <radsim_defines.hpp>
#include <stack>

// SystemC flit class to encapsulate different flit components. Flit payload pointers are recycled when a flit is no
// longer in use (i.e., freed) to avoid creating new pointers throughout the simulation time
class sc_flit {
 private:
  static std::stack<sc_bv<NOC_LINKS_PAYLOAD_WIDTH>*> _all;   // Stack of all payload pointers
  static std::stack<sc_bv<NOC_LINKS_PAYLOAD_WIDTH>*> _free;  // Stack of recycled free payload pointers

 public:
  bool _head;                                             // Is this a head flit?
  bool _tail;                                             // Is this a tail flit?
  Flit::FlitType _type;                                   // Transaction type
  sc_bv<NOC_LINKS_VCID_WIDTH> _vc_id;                     // Virtual channel ID this flit uses
  sc_bv<NOC_LINKS_PACKETID_WIDTH> _packet_id;             // Packet ID this flit belongs to
  sc_bv<NOC_LINKS_DEST_WIDTH> _dest;                      // Packet destination
  sc_bv<NOC_LINKS_PAYLOAD_WIDTH>* _payload;               // Payload data of the flit
  sc_bv<NOC_LINKS_DEST_INTERFACE_WIDTH> _dest_interface;  // Packet destination interface ID
  unsigned int _sim_transaction_id;                       // Unique simulation transaction ID for this flit

  sc_flit();
  sc_flit(const sc_flit& f);
  sc_flit(bool head, bool tail, Flit::FlitType type, unsigned int vc_id, const sc_uint<NOC_LINKS_DEST_WIDTH>& dest,
          const sc_bv<NOC_LINKS_DEST_INTERFACE_WIDTH>& dest_interface, const sc_bv<NOC_LINKS_PACKETID_WIDTH>& packet_id,
          unsigned int sim_transaction_id);
  ~sc_flit();
  static sc_bv<NOC_LINKS_PAYLOAD_WIDTH>* AllocateFlitPayload();
  void FreeFlit();
  static void FreeAllFlits();
  unsigned int GetStreamID();
};

// SystemC packet class
class sc_packet {
 private:
  std::vector<sc_flit> _flits;  // List of flits in this packet
  unsigned int _stream_id;      // Stream ID of this packet (to choose right adapter interface)

 public:
  sc_packet();
  unsigned int GetNumValidFlits();
  sc_flit* GetFlit(unsigned int idx);
  void AddFlit(sc_flit flit);
  void Reset();
  void SetStreamID(unsigned int id);
  unsigned int GetStreamID();
  void PrintFlitContents();
};

ostream& operator<<(ostream& os, sc_flit& flit);
ostream& operator<<(ostream& os, sc_packet& packet);