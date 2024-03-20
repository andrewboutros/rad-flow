#include <client.hpp>

client::client(const sc_module_name &name, unsigned int fifo_depth, RADSimDesignContext* radsim_design)
    : RADSimModule(name, radsim_design) {

  this->radsim_design = radsim_design; //AKB ADDED

  client_fifo_depth = fifo_depth;

  // Combinational logic and its sensitivity list
  SC_METHOD(Assign);
  sensitive << rst << client_fifo_full;
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

client::~client() {}

void client::Assign() {
  if (rst) {
    client_ready.write(true); // ready to accept requests from driver testbench
  } else {
    // Ready to accept new addend from driver testbench as long as the addend
    // FIFO is not full
    client_ready.write(!client_fifo_full.read());
  }
}

void client::Tick() {
  // Reset logic
  axis_client_interface.tvalid.write(false);
  while (!client_tdata_fifo.empty()) {
    client_tdata_fifo.pop();
  }
  client_fifo_full.write(false);
  wait();

  std::string src_port_name = module_name + ".axis_client_interface";

  // Always @ positive edge of the clock
  while (true) {
    // Interface with testbench driver
    if (client_ready.read() && client_valid.read()) {
      client_tdata_fifo.push(client_tdata);
      testbench_tlast = client_tlast.read();
      std::cout << module_name << ": Pushed request to FIFO" << std::endl;
    }
    client_fifo_full.write(client_tdata_fifo.size() >= client_fifo_depth);

    // Sending transactions to AXI-S NoC
    if (!client_tdata_fifo.empty()) {
      sc_bv<DATAW> tdata = client_tdata_fifo.front();
      std::string dst_port_name = "adder_inst.axis_adder_interface";
      //cout << dst_port_name << endl;
      uint64_t dst_addr = radsim_design->GetPortDestinationID(dst_port_name); //AKB changed to ptr deref
      uint64_t src_addr = radsim_design->GetPortDestinationID(src_port_name); //AKB changed to ptr deref

      axis_client_interface.tdest.write(dst_addr);
      axis_client_interface.tid.write(0);
      axis_client_interface.tstrb.write(0);
      axis_client_interface.tkeep.write(0);
      axis_client_interface.tuser.write(src_addr);
      axis_client_interface.tlast.write(testbench_tlast && (client_tdata_fifo.size() == 1));
      axis_client_interface.tdata.write(tdata);

      axis_client_interface.tvalid.write(true);
    } else {
      axis_client_interface.tvalid.write(false);
    }

    if (axis_client_interface.tvalid.read() &&
        axis_client_interface.tready.read()) {
      client_tdata_fifo.pop();
      int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
      std::cout << module_name << ": Sent Transaction! on cycle " << curr_cycle << std::endl;
    }
    wait();
  }
}

void client::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;

  port_name = module_name + ".axis_client_interface";
  RegisterAxisMasterPort(port_name, &axis_client_interface, DATAW, 0);
}
