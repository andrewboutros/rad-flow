#include <sc_flit.hpp>

std::stack<sc_bv<NOC_LINKS_PAYLOAD_WIDTH>*> sc_flit::_all;
std::stack<sc_bv<NOC_LINKS_PAYLOAD_WIDTH>*> sc_flit::_free;

sc_flit::sc_flit() {
  _head = false;
  _tail = false;
  _type = Flit::READ_REQUEST;
  _vc_id = 0;
  _dest = 0;
  _dest_interface = 0;
  _packet_id = 0;
  _payload = AllocateFlitPayload();
}

sc_flit::sc_flit(bool head, bool tail, Flit::FlitType type, int vc_id, const sc_uint<NOC_LINKS_DEST_WIDTH>& dest,
                 const sc_bv<NOC_LINKS_DEST_INTERFACE_WIDTH>& dest_interface,
                 const sc_bv<NOC_LINKS_PACKETID_WIDTH>& packet_id, int sim_transaction_id) {
  _head = head;
  _tail = tail;
  _type = type;
  _vc_id = vc_id;
  _dest = dest;
  _dest_interface = dest_interface;
  _packet_id = packet_id;
  _payload = AllocateFlitPayload();
  _sim_transaction_id = sim_transaction_id;
}

sc_flit::sc_flit(const sc_flit& f) {
  _head = f._head;
  _tail = f._tail;
  _type = f._type;
  _vc_id = f._vc_id;
  _dest = f._dest;
  _dest_interface = f._dest_interface;
  _packet_id = f._packet_id;
  _payload = AllocateFlitPayload();
  *_payload = *(f._payload);
  _sim_transaction_id = f._sim_transaction_id;
}

sc_bv<NOC_LINKS_PAYLOAD_WIDTH>* sc_flit::AllocateFlitPayload() {
  sc_bv<NOC_LINKS_PAYLOAD_WIDTH>* payload_ptr;
  if (_free.empty()) {
    payload_ptr = new sc_bv<NOC_LINKS_PAYLOAD_WIDTH>();
    _all.push(payload_ptr);
  } else {
    payload_ptr = _free.top();
    _free.pop();
  }
  return payload_ptr;
}

void sc_flit::FreeFlit() { _free.push(_payload); }

void sc_flit::FreeAllFlits() {
  while (!_all.empty()) {
    delete _all.top();
    _all.pop();
  }
}

int sc_flit::GetStreamID() { return _vc_id.to_uint(); }

sc_packet::sc_packet() {
  _stream_id = 0;
}

int sc_packet::GetNumValidFlits() { 
  return _flits.size(); 
}

sc_flit* sc_packet::GetFlit(int idx) {
  assert(idx < _flits.size());
  return &_flits[idx];
}

void sc_packet::AddFlit(sc_flit flit) {
  _flits.push_back(flit);
}

void sc_packet::Reset() {
  _flits.clear();
  _stream_id = 0;
}

void sc_packet::SetStreamID(int id) { 
  _stream_id = id; 
}

int sc_packet::GetStreamID() { 
  return _stream_id; 
}

void sc_packet::PrintFlitContents() { 
  for (unsigned int flit_id = 0; flit_id < _flits.size(); flit_id++)
    cerr << *(_flits[flit_id]._payload);
  if (_flits.size() != 0) cerr << endl;
}

std::ostream& operator<<(std::ostream& os, sc_flit& flit) {
  os << "|" << flit._head << "|" << flit._tail << "|" << flit._vc_id << "|" << flit._packet_id << "|"
     << "|" << flit._payload->to_string(SC_HEX) << "|" << std::dec;
  return os;
}

std::ostream& operator<<(std::ostream& os, sc_packet& packet) {
  if (packet.GetNumValidFlits() > 0) {
    os << "Pacekt " << packet.GetFlit(0)->_packet_id.to_uint() << " has " << packet.GetNumValidFlits()
       << " valid flits";
  }
  return os;
}