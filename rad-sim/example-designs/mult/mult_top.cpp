#include <mult_top.hpp>

mult_top::mult_top(const sc_module_name &name, RADSimDesignContext* radsim_design)
    : sc_module(name) {

  std::string module_name_str;
  char module_name[25];

  module_name_str = "client_inst";
  std::strcpy(module_name, module_name_str.c_str());

  client_inst = new client_mult(module_name, 16, radsim_design); //AKB added last arg
  client_inst->rst(rst);
  client_inst->client_tdata(client_tdata);
  client_inst->client_tlast(client_tlast);
  client_inst->client_valid(client_valid);
  client_inst->client_ready(client_ready);

  module_name_str = "mult_inst";
  std::strcpy(module_name, module_name_str.c_str());
  mult_inst = new mult(module_name, radsim_design);
  mult_inst->rst(rst);
  mult_inst->response(response);
  mult_inst->response_valid(response_valid);
  mult_inst->mult_inter_rad_recvd(this->mult_inter_rad_recvd);

  //AKB: added code block for portal module
  module_name_str = "portal_inst";
  std::strcpy(module_name, module_name_str.c_str());
  portal_inst = new portal_mult(module_name, radsim_design);
  portal_inst->rst(rst);

  //connect master to master instead, to expose to top
  portal_inst->portal_axis_master.ConnectToPort(this->design_top_portal_axis_master);
  portal_inst->portal_axis_slave.ConnectToPort(this->design_top_portal_axis_slave);

  radsim_design->BuildDesignContext("mult.place", "mult.clks");
  radsim_design->CreateSystemNoCs(rst);
  radsim_design->ConnectModulesToNoC();
}

  mult_top::~mult_top() {
  delete mult_inst;
  delete client_inst;
}