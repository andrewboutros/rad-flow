#include <client.hpp>

client::client(const sc_module_name &name, unsigned int fifo_depth, RADSimDesignContext* radsim_design)
    : RADSimModule(name, radsim_design) {

  this->radsim_design = radsim_design;

  char fifo_name[25];
  std::string fifo_name_str;
  fifo_name_str = "client_tdata_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  client_tdata_fifo = new fifo<sc_bv<DATAW>>(fifo_name, FIFO_DEPTH, FIFO_DEPTH - 1, 0);
  client_tdata_fifo->clk(clk);
  client_tdata_fifo->rst(rst);
  client_tdata_fifo->wen(client_tdata_fifo_wen_signal);
  client_tdata_fifo->ren(client_tdata_fifo_ren_signal);
  client_tdata_fifo->wdata(client_tdata);
  client_tdata_fifo->full(client_tdata_fifo_full_signal);
  client_tdata_fifo->almost_full(client_tdata_fifo_almost_full_signal);
  client_tdata_fifo->empty(client_tdata_fifo_empty_signal);
  client_tdata_fifo->almost_empty(client_tdata_fifo_almost_empty_signal);
  client_tdata_fifo->rdata(client_tdata_fifo_rdata_signal);

  fifo_name_str = "client_tlast_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  client_tlast_fifo = new fifo<bool>(fifo_name, FIFO_DEPTH, FIFO_DEPTH - 1, 0);
  client_tlast_fifo->clk(clk);
  client_tlast_fifo->rst(rst);
  client_tlast_fifo->wen(client_tlast_fifo_wen_signal);
  client_tlast_fifo->ren(client_tlast_fifo_ren_signal);
  client_tlast_fifo->wdata(client_tlast);
  client_tlast_fifo->full(client_tlast_fifo_full_signal);
  client_tlast_fifo->almost_full(client_tlast_fifo_almost_full_signal);
  client_tlast_fifo->empty(client_tlast_fifo_empty_signal);
  client_tlast_fifo->almost_empty(client_tlast_fifo_almost_empty_signal);
  client_tlast_fifo->rdata(client_tlast_fifo_rdata_signal);

  // Combinational logic and its sensitivity list
  SC_METHOD(Assign);
  sensitive << rst << client_ready << client_valid << client_tdata_fifo_almost_full_signal
    << client_tdata_fifo_empty_signal << axis_client_interface.tready 
    << axis_client_interface.tvalid << client_tdata_fifo_rdata_signal 
    << client_tlast_fifo_rdata_signal;
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

client::~client() {
  delete client_tdata_fifo;
  delete client_tlast_fifo;
}

void client::Tick() {
  wait();
  while (true) {
    if (client_ready.read() && client_valid.read()) {
      // std::cout << this->name() << " @ cycle " 
      //   << GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period")) << ": "
      //   << " Pushed request to FIFO!" << std::endl;
    }

    if (axis_client_interface.tvalid.read() && axis_client_interface.tready.read()) {
      // std::cout << this->name() << " @ cycle " 
      //   << GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period")) << ": "
      //   << " Sent Transaction!" << std::endl;
    }
    wait();
  }
}

void client::Assign() {
  if (rst) {
    client_tdata_fifo_wen_signal.write(false);
    client_tlast_fifo_wen_signal.write(false);
    client_ready.write(false);
    axis_client_interface.tvalid.write(false);
  } else {
    if (!client_tdata_fifo_empty_signal.read()) {
      sc_bv<DATAW> tdata = client_tdata_fifo_rdata_signal.read();
      bool tlast = client_tlast_fifo_rdata_signal.read();
      std::string src_port_name = module_name + ".axis_client_interface";
      std::string dst_port_name = "adder_inst.axis_adder_interface";
      //cout << dst_port_name << endl;
      uint64_t dst_addr = radsim_design->GetPortDestinationID(dst_port_name); //AKB changed to ptr deref
      uint64_t src_addr = radsim_design->GetPortDestinationID(src_port_name); //AKB changed to ptr deref

      axis_client_interface.tdest.write(dst_addr);
      axis_client_interface.tid.write(0);
      axis_client_interface.tstrb.write(0);
      axis_client_interface.tkeep.write(0);
      axis_client_interface.tuser.write(src_addr);
      axis_client_interface.tlast.write(tlast);
      axis_client_interface.tdata.write(tdata);
      axis_client_interface.tvalid.write(true);
    } else {
      axis_client_interface.tvalid.write(false);
    }

    client_ready.write(!client_tdata_fifo_almost_full_signal.read());

    client_tdata_fifo_wen_signal.write(client_ready.read() && client_valid.read());
    client_tlast_fifo_wen_signal.write(client_ready.read() && client_valid.read());

    client_tdata_fifo_ren_signal.write(axis_client_interface.tvalid.read() &&
      axis_client_interface.tready.read());
    client_tlast_fifo_ren_signal.write(axis_client_interface.tvalid.read() &&
      axis_client_interface.tready.read());
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
