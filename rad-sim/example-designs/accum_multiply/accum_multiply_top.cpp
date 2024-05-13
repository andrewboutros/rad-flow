#include <aximm_hello_world_top.hpp>

aximm_hello_world_top::aximm_hello_world_top(const sc_module_name &name)
    : sc_module(name) {

  std::string module_name_str;
  char module_name[25];

  module_name_str = "requester_inst";
  std::strcpy(module_name, module_name_str.c_str());

  requester_inst = new requester(module_name, 16);
  requester_inst->rst(rst);
  requester_inst->req_addr(req_addr);
  requester_inst->req_type(req_type);
  requester_inst->req_wdata(req_wdata);
  requester_inst->req_valid(req_valid);
  requester_inst->req_ready(req_ready);
  requester_inst->received_responses(received_responses);

  module_name_str = "responder_inst";
  std::strcpy(module_name, module_name_str.c_str());
  responder_inst = new responder(module_name, 16);
  responder_inst->rst(rst);

  radsim_design.BuildDesignContext("aximm_hello_world.place",
                                   "aximm_hello_world.clks");
  radsim_design.CreateSystemNoCs(rst);
  radsim_design.ConnectModulesToNoC();
}

aximm_hello_world_top::~aximm_hello_world_top() {
  delete requester_inst;
  delete responder_inst;
}