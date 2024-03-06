#include <add_top.hpp>

add_top::add_top(const sc_module_name &name, RADSimDesignContext* radsim_design)
    : sc_module(name) {

  std::string module_name_str;
  char module_name[25];

  module_name_str = "client_inst";
  std::strcpy(module_name, module_name_str.c_str());

  client_inst = new client(module_name, 16, radsim_design); //AKB added last arg
  client_inst->rst(rst);
  client_inst->client_tdata(client_tdata);
  client_inst->client_tlast(client_tlast);
  client_inst->client_valid(client_valid);
  client_inst->client_ready(client_ready);

  module_name_str = "adder_inst";
  std::strcpy(module_name, module_name_str.c_str());
  adder_inst = new adder(module_name, radsim_design); //AKB added last arg
  adder_inst->rst(rst);
  adder_inst->response(response);
  adder_inst->response_valid(response_valid);

  //AKB: added code block for portal module
  module_name_str = "portal_inst";
  std::strcpy(module_name, module_name_str.c_str());
  portal_inst = new portal(module_name, radsim_design);
  portal_inst->portal_in(this->portal_in); //connecting portal's portal_in to the parent class system_top's portal_in
  portal_inst->portal_out(this->portal_out);
  //this->top_axis_portal_interface = &(portal_inst->axis_add_portal_slave_interface);
  portal_inst->portal_recvd(this->portal_recvd);

  sig_portal_master_design_slave.Connect(portal_inst->portal_axis_master, this->design_top_portal_axis_slave); //Connect(axis_master_port &m, axis_slave_port &s)
  sig_design_master_portal_slave.Connect(this->design_top_portal_axis_master, portal_inst->portal_axis_slave);
  //master to slave port connections
  /*portal_inst->portal_axis_master.tvalid(sig_portal_master_design_slave.tvalid);
  portal_inst->portal_axis_master.tready(sig_portal_master_design_slave.tready);
  portal_inst->portal_axis_master.tdata(sig_portal_master_design_slave.tdata);
  portal_inst->portal_axis_master.tstrb(sig_portal_master_design_slave.tstrb);
  portal_inst->portal_axis_master.tkeep(sig_portal_master_design_slave.tkeep);
  portal_inst->portal_axis_master.tlast(sig_portal_master_design_slave.tlast);
  portal_inst->portal_axis_master.tid(sig_portal_master_design_slave.tid);
  portal_inst->portal_axis_master.tdest(sig_portal_master_design_slave.tdest);
  portal_inst->portal_axis_master.tuser(sig_portal_master_design_slave.tuser);*/
  /*this->design_top_portal_axis_master.tvalid(sig_portal_master_design_slave.tvalid);
  this->design_top_portal_axis_master.tready(sig_portal_master_design_slave.tready);
  this->design_top_portal_axis_master.tdata(sig_portal_master_design_slave.tdata);
  this->design_top_portal_axis_master.tstrb(sig_portal_master_design_slave.tstrb);
  this->design_top_portal_axis_master.tkeep(sig_portal_master_design_slave.tkeep);
  this->design_top_portal_axis_master.tlast(sig_portal_master_design_slave.tlast);
  this->design_top_portal_axis_master.tid(sig_portal_master_design_slave.tid);
  this->design_top_portal_axis_master.tdest(sig_portal_master_design_slave.tdest);
  this->design_top_portal_axis_master.tuser(sig_portal_master_design_slave.tuser);
  //slave to master port connections
  this->design_top_portal_axis_slave.tvalid(sig_portal_master_design_slave.tvalid);
  this->design_top_portal_axis_slave.tready(sig_portal_master_design_slave.tready);
  this->design_top_portal_axis_slave.tdata(sig_portal_master_design_slave.tdata);
  this->design_top_portal_axis_slave.tstrb(sig_portal_master_design_slave.tstrb);
  this->design_top_portal_axis_slave.tkeep(sig_portal_master_design_slave.tkeep);
  this->design_top_portal_axis_slave.tlast(sig_portal_master_design_slave.tlast);
  this->design_top_portal_axis_slave.tid(sig_portal_master_design_slave.tid);
  this->design_top_portal_axis_slave.tdest(sig_portal_master_design_slave.tdest);
  this->design_top_portal_axis_slave.tuser(sig_portal_master_design_slave.tuser);*/
  //would need to repeat for other slave/master pair

  radsim_design->BuildDesignContext("/home/bassiabn/rad-sim/rad-flow/rad-sim/example-designs/add", "add.place",
                                   "add.clks"); //AKB changed to ptr deref and added first arg
  radsim_design->CreateSystemNoCs(rst); //AKB changed to ptr deref
  radsim_design->ConnectModulesToNoC(); //AKB changed to ptr deref
}

add_top::~add_top() {
  delete adder_inst;
  delete client_inst;
}