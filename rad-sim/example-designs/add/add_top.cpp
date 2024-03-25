#include <add_top.hpp>

add_top::add_top(const sc_module_name &name, RADSimDesignContext* radsim_design)
    : sc_module(name) {

  std::string module_name_str;
  char module_name[25];

  module_name_str = "client_inst";
  std::strcpy(module_name, module_name_str.c_str());

  client_inst = new client(module_name, 16, radsim_design);
  client_inst->rst(rst);
  client_inst->client_tdata(client_tdata);
  client_inst->client_tlast(client_tlast);
  client_inst->client_valid(client_valid);
  client_inst->client_ready(client_ready);

  module_name_str = "adder_inst";
  std::strcpy(module_name, module_name_str.c_str());
  adder_inst = new adder(module_name, radsim_design);
  adder_inst->rst(rst);
  adder_inst->response(response);
  adder_inst->response_valid(response_valid);

  //create portal module
  module_name_str = "portal_inst";
  std::strcpy(module_name, module_name_str.c_str());
  portal_inst = new portal(module_name, radsim_design);
  portal_inst->portal_recvd(this->portal_recvd);

  //connect master to master instead, to expose to top
  portal_inst->portal_axis_master.ConnectToPort(this->design_top_portal_axis_master);
  portal_inst->portal_axis_slave.ConnectToPort(this->design_top_portal_axis_slave); //top drives portal bc top receives slave inputs

  radsim_design->BuildDesignContext("/home/bassiabn/rad-sim/rad-flow/rad-sim/example-designs/add", "add.place",
                                   "add.clks");
  radsim_design->CreateSystemNoCs(rst);
  radsim_design->ConnectModulesToNoC();
}

add_top::~add_top() {
  delete adder_inst;
  delete client_inst;
}