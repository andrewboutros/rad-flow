#include <adder.hpp>

adder::adder(const sc_module_name &name, RADSimDesignContext* radsim_design) //AKB added last arg
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

adder::~adder() {}

void adder::Assign() {
  if (rst) {
    adder_rolling_sum = 0;
    //axis_adder_interface.tready.write(false); //moving to Tick() bc needs to toggle with clock cycles
  } else {
    // Always ready to accept the transaction
    //axis_adder_interface.tready.write(true);
  }
}

void adder::Tick() {
  response_valid.write(0);
  response.write(0);
  axis_adder_interface.tready.write(false);
  wait();

  int count_sent_addends = 0;
  //int total_num_addends = 10;
  //bool accept_data = true;
  int accept_data = 0;
  int accept_delay = 0; //change this to experiment with delaying acceptance of data from NoC
  // Always @ positive edge of the clock
  int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
  std::cout << "adder.cpp while loop at cycle " << curr_cycle << std::endl;
  while (true) {
    //Toggle ready signal
    if (accept_data == accept_delay) {
      axis_adder_interface.tready.write(true);
      accept_data = 0;
    }
    else {
      axis_adder_interface.tready.write(false);
      accept_data++;
    }

    //std::cout << "tready: " << axis_adder_master_interface.tready.read() << std::endl;

    //accept_data = !accept_data;
    // Receiving transaction from AXI-S interface
    if (axis_adder_interface.tvalid.read() &&
        axis_adder_interface.tready.read()
        //also check master since sending on master in same cycle
        //in future if needed, could decouple receiving and sending using a fifo
        //&& axis_adder_master_interface.tready.read() //this is input from NoC
    ){
      uint64_t current_sum = adder_rolling_sum.to_uint64();
      adder_rolling_sum = current_sum + axis_adder_interface.tdata.read().to_uint64();
      t_finished.write(axis_adder_interface.tlast.read());
      int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
      std::cout << module_name << ": Got Transaction " << count_sent_addends << " on cycle " << curr_cycle <<" (user = "
                << axis_adder_interface.tuser.read().to_uint64() << ") (addend = "
                << axis_adder_interface.tdata.read().to_uint64() << ")!"
                << std::endl;
        
        adder_tdata_tlast_fifo.push(std::make_tuple(axis_adder_interface.tdata.read(), axis_adder_interface.tlast.read()));
    }

    if (adder_tdata_tlast_fifo.size() > 0) { //fifo not empty
        //TODO: restrict fifo size, not doing so for now
        std::string src_port_name = module_name + ".axis_adder_master_interface";
        std::string dst_port_name = "portal_inst.axis_portal_slave_interface";
        cout << axis_adder_interface.tdata.read().to_uint64() << endl;
        uint64_t dst_addr = radsim_design->GetPortDestinationID(dst_port_name); //AKB changed to ptr deref
        uint64_t src_addr = radsim_design->GetPortDestinationID(src_port_name); //AKB changed to ptr deref
        axis_adder_master_interface.tdest.write(dst_addr);
        axis_adder_master_interface.tid.write(0);
        axis_adder_master_interface.tstrb.write(0);
        axis_adder_master_interface.tkeep.write(0);
        axis_adder_master_interface.tuser.write(src_addr);
        axis_adder_master_interface.tlast.write(std::get<1>(adder_tdata_tlast_fifo.front())); //true only for last addend
        axis_adder_master_interface.tdata.write(std::get<0>(adder_tdata_tlast_fifo.front()));
        axis_adder_master_interface.tvalid.write(true);
    }
    else {
      axis_adder_master_interface.tvalid.write(false);
    }

    //for testing: checking handshaking
    int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
    if (curr_cycle < 200) {
      std::cout << "axis_adder_master_interface.tvalid.read() " << axis_adder_master_interface.tvalid.read() << " on cycle " << curr_cycle << std::endl;
      std::cout << "axis_adder_master_interface.tready.read() " << axis_adder_master_interface.tready.read() << " on cycle " << curr_cycle << std::endl;
    }
    //sent to portal module
    if (axis_adder_master_interface.tvalid.read() && axis_adder_master_interface.tready.read()) {
        count_sent_addends++;
        std::cout << "Sent the " << count_sent_addends << "th addend over NoC to portal module " << std::endl;
        adder_tdata_tlast_fifo.pop();
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

  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;

  port_name = module_name + ".axis_adder_master_interface";
  RegisterAxisMasterPort(port_name, &axis_adder_master_interface, DATAW, 0);
}