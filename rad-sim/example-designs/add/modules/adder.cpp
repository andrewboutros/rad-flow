#include <adder.hpp>

adder::adder(const sc_module_name &name)
    : RADSimModule(name) {

  // Combinational logic and its sensitivity list
  SC_METHOD(Assign);
  sensitive << rst;
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

adder::~adder() {}

void adder::Assign() {
  if (rst) {
    adder_rolling_sum = 0;
    axis_adder_interface.tready.write(false);
  } else {
    // Always ready to accept the transaction
    axis_adder_interface.tready.write(true);
  }
}

void adder::Tick() {
  response_valid.write(0);
  response.write(0);
  wait();

  // Always @ positive edge of the clock
  while (true) {
    // Receiving transaction from AXI-S interface
    if (axis_adder_interface.tvalid.read() &&
        axis_adder_interface.tready.read()) {
      uint64_t current_sum = adder_rolling_sum.to_uint64();
      adder_rolling_sum = current_sum + axis_adder_interface.tdata.read().to_uint64();
      t_finished.write(axis_adder_interface.tlast.read());
      //std::cout << module_name << ": Got Transaction (user = "
      //          << axis_adder_interface.tuser.read().to_uint64() << ") (addend = "
      //          << axis_adder_interface.tdata.read().to_uint64() << ")!"
      //          << std::endl;
    }

    // Print Sum and Exit
    if (t_finished.read()) {
        response_valid.write(1);
        response.write(adder_rolling_sum);
    }
    wait();
  }
}

void adder::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;

  port_name = module_name + ".axis_adder_interface";
  RegisterAxisSlavePort(port_name, &axis_adder_interface, DATAW, 0);
}