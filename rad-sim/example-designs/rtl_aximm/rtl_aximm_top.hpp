#pragma once

#include <radsim_config.hpp>
#include <client.hpp>
#include <server.hpp>
#include <systemc.h>
#include <vector>

class rtl_aximm_top : public sc_module {
private:
  client *client_inst;
  server *server_inst;

public:
  sc_in<bool> rst;
  // Client Interface
  sc_in<bool> start_sig;
	sc_out<bool> start_rdy;
	sc_out<bool> output_valid;
	sc_out<bool> output_result;
	sc_in<bool> output_rdy;

  rtl_aximm_top(const sc_module_name &name);
  ~rtl_aximm_top();
};