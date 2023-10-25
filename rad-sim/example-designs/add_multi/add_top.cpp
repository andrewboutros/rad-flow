#include <add_top.hpp>

add_top::add_top(const sc_module_name &name)
    : sc_module(name) {

  std::string module_name_str;
  char module_name[25];

  module_name_str = "client_inst";
  std::strcpy(module_name, module_name_str.c_str());

  client_inst = new client(module_name, 16);
  client_inst->rst(rst);
  client_inst->client_tdata(client_tdata);
  client_inst->client_tlast(client_tlast);
  client_inst->client_valid(client_valid);
  client_inst->client_ready(client_ready);

  module_name_str = "adder_inst";
  std::strcpy(module_name, module_name_str.c_str());
  adder_inst = new adder(module_name);
  adder_inst->rst(rst);
  adder_inst->response(response);
  adder_inst->response_valid(response_valid);

  module_name_str = "adder_inst2";
  std::strcpy(module_name, module_name_str.c_str());
  adder_inst2 = new adder(module_name);
  adder_inst2->rst(rst);
  adder_inst2->response(response);
  adder_inst2->response_valid(response_valid);

  radsim_design.BuildDesignContext("add.place",
                                   "add.clks");
  radsim_design.CreateSystemNoCs(rst);
  radsim_design.ConnectModulesToNoC();
}

add_top::~add_top() {
  delete adder_inst;
  delete adder_inst2;
  delete client_inst;
}
