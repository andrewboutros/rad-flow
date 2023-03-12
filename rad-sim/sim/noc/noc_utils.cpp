#include <noc_utils.hpp>

int VCIDFromType(Flit::FlitType type, BookSimConfig *config) {
  if (type == Flit::READ_REQUEST) {
    return config->GetInt("read_request_begin_vc");
  } else if (type == Flit::WRITE_REQUEST) {
    return config->GetInt("write_request_begin_vc");
  } else if (type == Flit::WRITE_DATA) {
    return config->GetInt("write_data_begin_vc");
  } else if (type == Flit::READ_REPLY) {
    return config->GetInt("read_reply_begin_vc");
  } else {
    return config->GetInt("write_reply_begin_vc");
  }
}

void set_flit_payload(sc_flit &packetization_flit,
                      sc_bv<AXI_TRANSACTION_MAX_WIDTH> &packet_bv,
                      int flit_id) {
  unsigned int start_idx = flit_id * NOC_LINKS_PAYLOAD_WIDTH;
  unsigned int end_idx = std::min((flit_id + 1) * NOC_LINKS_PAYLOAD_WIDTH,
                                  AXIS_TRANSACTION_PAYLOAD_WIDTH);
  *(packetization_flit._payload) = packet_bv.range(end_idx - 1, start_idx);
}

void set_flit_payload(sc_flit &packetization_flit,
                      sc_bv<AXIS_TRANSACTION_WIDTH> &packet_bv, int flit_id) {
  unsigned int start_idx = flit_id * NOC_LINKS_PAYLOAD_WIDTH;
  unsigned int end_idx = std::min((flit_id + 1) * NOC_LINKS_PAYLOAD_WIDTH,
                                  AXIS_TRANSACTION_PAYLOAD_WIDTH);
  *(packetization_flit._payload) = packet_bv.range(end_idx - 1, start_idx);
}