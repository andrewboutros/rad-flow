#include <noc_utils.hpp>

int VCIDFromType(Flit::FlitType type, BookSimConfig* config){
  if (type == Flit::READ_REQUEST){
    return config->GetInt("read_request_begin_vc");
  } else if (type == Flit::WRITE_REQUEST){
    return config->GetInt("write_request_begin_vc");
  } else if (type == Flit::WRITE_DATA){
    return config->GetInt("write_data_begin_vc");
  } else if (type == Flit::READ_REPLY){
    return config->GetInt("read_reply_begin_vc");
  } else {
    return config->GetInt("write_reply_begin_vc");
  }
}