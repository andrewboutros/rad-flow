#include <adder_wrapper.hpp>
#include <Vadder.h>

adder_wrapper::adder_wrapper(const sc_module_name &name) : radsim_module(name) {
  Vadder* vadder = new Vadder{"vadder"};
  vadder->clk(clk);
  vadder->rst(rst);
  vadder->axis_adder_interface_tvalid(axis_adder_interface.tvalid);
  vadder->axis_adder_interface_tready(axis_adder_interface.tready);
  vadder->axis_adder_interface_tlast(axis_adder_interface.tlast);
  vadder->axis_adder_interface_tdata(axis_adder_interface.tdata);

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

adder_wrapper::~adder_wrapper() {}

void adder_wrapper::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;

  port_name = module_name + ".axis_adder_interface";
  RegisterAxisSlavePort(port_name, &axis_adder_interface, DATAW, 0);
}