#include <rtl_aximm_top.hpp>

rtl_aximm_top::rtl_aximm_top(const sc_module_name &name)
    : sc_module(name) {

  std::string module_name_str;
  char module_name[25];

  module_name_str = "client_inst";
  std::strcpy(module_name, module_name_str.c_str());

  client_inst = new client(module_name);
  client_inst->rst(rst);
  client_inst->start_sig(start_sig);
	client_inst->start_rdy(start_rdy);
	client_inst->output_valid(output_valid);
	client_inst->output_result(output_result);
	client_inst->output_rdy(output_rdy);

  module_name_str = "server_inst";
  std::strcpy(module_name, module_name_str.c_str());

  server_inst = new server(module_name);
  server_inst->rst(rst);

  radsim_design.BuildDesignContext("rtl_aximm.place",
                                   "rtl_aximm.clks");
  radsim_design.CreateSystemNoCs(rst);
  radsim_design.ConnectModulesToNoC();
}

rtl_aximm_top::~rtl_aximm_top() {
  delete client_inst;
}