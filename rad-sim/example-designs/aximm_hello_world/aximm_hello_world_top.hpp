#pragma once

#include <radsim_config.hpp>
#include <requester.hpp>
#include <responder.hpp>
#include <sim_utils.hpp>
#include <systemc.h>
#include <vector>

class aximm_hello_world_top : public sc_module {
private:
  requester *requester_inst;
  responder *responder_inst;

public:
  sc_in<bool> rst;
  // Requester's interface
  sc_in<uint64_t> req_addr;
  sc_in<bool> req_type;
  sc_in<sc_bv<DATAW>> req_wdata;
  sc_in<bool> req_valid;
  sc_out<bool> req_ready;

  aximm_hello_world_top(const sc_module_name &name);
  ~aximm_hello_world_top();
};