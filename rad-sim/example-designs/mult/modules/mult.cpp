#include <mult.hpp>

mult::mult(const sc_module_name &name, RADSimDesignContext* radsim_design) //AKB added last arg
    : RADSimModule(name, radsim_design) {
  
  this->radsim_design = radsim_design;

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

mult::~mult() {}

void mult::Assign() {
  if (rst) {
    mult_rolling_product = 1;
    axis_mult_interface.tready.write(false);
  } else {
    // Always ready to accept the transaction
    axis_mult_interface.tready.write(true);
  }
}

void mult::Tick() {
  response_valid.write(0);
  response.write(0);
  wait();
  bool printed_end_cycle = false;


  // Always @ positive edge of the clock
  while (true) {
    // Receiving transaction from AXI-S interface
    if (axis_mult_interface.tvalid.read() &&
        axis_mult_interface.tready.read()) {
      uint64_t current_product = mult_rolling_product.to_uint64(); //removing for experiment
      mult_rolling_product = current_product * axis_mult_interface.tdata.read().to_uint64(); //removing for experiment
      t_finished.write(axis_mult_interface.tlast.read());
      // std::cout << module_name << ": Got Transaction (user = "
      //           << axis_mult_interface.tuser.read().to_uint64() << ") (factor = "
      //           << axis_mult_interface.tdata.read().to_uint64() << ")!"
      //           << std::endl;
    }

    // Print Sum and Exit
    if (t_finished.read()) {
        response_valid.write(1);
        response.write(mult_rolling_product);
        //mult_inter_rad_recvd.write(1); //maybe not needed if using the
        if (!printed_end_cycle) {
          //int end_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
          //std::cout << "mult.cpp received all factors from add RAD at cycle " << end_cycle << std::endl;
          printed_end_cycle = true;
        }
    }
    wait();
  }
}

void mult::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;

  port_name = module_name + ".axis_mult_interface";
  RegisterAxisSlavePort(port_name, &axis_mult_interface, DATAW, 0);
}