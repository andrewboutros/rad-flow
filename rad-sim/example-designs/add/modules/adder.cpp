#include <adder.hpp>

adder::adder(const sc_module_name &name, RADSimDesignContext* radsim_design) //AKB added last arg
    : RADSimModule(name, radsim_design) {

  this->radsim_design = radsim_design;

  char fifo_name[25];
  std::string fifo_name_str;
  fifo_name_str = "adder_tdata_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  adder_tdata_fifo = new fifo<sc_bv<DATAW>>(fifo_name, FIFO_DEPTH, FIFO_DEPTH - 1, 0);
  adder_tdata_fifo->clk(clk);
  adder_tdata_fifo->rst(rst);
  adder_tdata_fifo->wen(adder_tdata_fifo_wen_signal);
  adder_tdata_fifo->ren(adder_tdata_fifo_ren_signal);
  //sc_in<sc_bv<128>> my_subset;
  //my_subset = axis_adder_interface.tdata.read().range(DATAW-1, 0);
  //adder_tdata_fifo->wdata(my_subset);
  //adder_tdata_fifo->wdata(axis_adder_interface.tdata.read().range(DATAW-1, 0));
  adder_tdata_fifo->wdata(adder_tdata);
  adder_tdata_fifo->full(adder_tdata_fifo_full_signal);
  adder_tdata_fifo->almost_full(adder_tdata_fifo_almost_full_signal);
  adder_tdata_fifo->empty(adder_tdata_fifo_empty_signal);
  adder_tdata_fifo->almost_empty(adder_tdata_fifo_almost_empty_signal);
  adder_tdata_fifo->rdata(adder_tdata_fifo_rdata_signal);

  fifo_name_str = "adder_tlast_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  adder_tlast_fifo = new fifo<bool>(fifo_name, FIFO_DEPTH, FIFO_DEPTH - 1, 0);
  adder_tlast_fifo->clk(clk);
  adder_tlast_fifo->rst(rst);
  adder_tlast_fifo->wen(adder_tlast_fifo_wen_signal);
  adder_tlast_fifo->ren(adder_tlast_fifo_ren_signal);
  adder_tlast_fifo->wdata(axis_adder_interface.tlast);
  adder_tlast_fifo->full(adder_tlast_fifo_full_signal);
  adder_tlast_fifo->almost_full(adder_tlast_fifo_almost_full_signal);
  adder_tlast_fifo->empty(adder_tlast_fifo_empty_signal);
  adder_tlast_fifo->almost_empty(adder_tlast_fifo_almost_empty_signal);
  adder_tlast_fifo->rdata(adder_tlast_fifo_rdata_signal);

  // Combinational logic and its sensitivity list
  SC_METHOD(Assign);
  sensitive << rst << axis_adder_interface.tready
    << axis_adder_interface.tvalid << adder_tdata_fifo_almost_full_signal
    << adder_tdata_fifo_empty_signal << axis_adder_master_interface.tready 
    << axis_adder_master_interface.tvalid << adder_tdata_fifo_rdata_signal 
    << adder_tlast_fifo_rdata_signal;
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

adder::~adder() {
  delete adder_tdata_fifo;
  delete adder_tlast_fifo;
}

void adder::Assign() {
  if (rst) {
    adder_rolling_sum = 0;
    axis_adder_interface.tready.write(false);
    adder_tdata_fifo_wen_signal.write(false);
    adder_tlast_fifo_wen_signal.write(false);
    axis_adder_master_interface.tvalid.write(false);
    adder_tdata.write(0);
  } else {
    if (!adder_tdata_fifo_empty_signal.read()) {
      sc_bv<DATAW> tdata = adder_tdata_fifo_rdata_signal.read();
      bool tlast = adder_tlast_fifo_rdata_signal.read();
      std::string src_port_name = module_name + ".axis_adder_master_interface";
      std::string dst_port_name = "portal_inst.axis_portal_slave_interface";
      cout << dst_port_name << endl;
      uint64_t dst_addr = radsim_design->GetPortDestinationID(dst_port_name); //AKB changed to ptr deref
      uint64_t src_addr = radsim_design->GetPortDestinationID(src_port_name); //AKB changed to ptr deref
      axis_adder_master_interface.tdest.write(dst_addr);
      axis_adder_master_interface.tid.write(0);
      axis_adder_master_interface.tstrb.write(0);
      axis_adder_master_interface.tkeep.write(0);
      axis_adder_master_interface.tuser.write(src_addr);
      axis_adder_master_interface.tlast.write(tlast);
      axis_adder_master_interface.tdata.write(tdata);
      axis_adder_master_interface.tvalid.write(true);
      adder_tdata.write(axis_adder_interface.tdata.read());
    } else {
      // Always ready to accept the transaction
      //axis_adder_interface.tready.write(true);
      axis_adder_master_interface.tvalid.write(false);
      adder_tdata.write(axis_adder_interface.tdata.read());
    }

    axis_adder_interface.tready.write(!adder_tdata_fifo_almost_full_signal.read());

    adder_tdata_fifo_wen_signal.write(axis_adder_interface.tready.read() && axis_adder_interface.tvalid.read());
    adder_tlast_fifo_wen_signal.write(axis_adder_interface.tready.read() && axis_adder_interface.tvalid.read());

    adder_tdata_fifo_ren_signal.write(axis_adder_master_interface.tvalid.read() &&
      axis_adder_master_interface.tready.read());
    adder_tlast_fifo_ren_signal.write(axis_adder_master_interface.tvalid.read() &&
      axis_adder_master_interface.tready.read());
  }
}

void adder::Tick() {
  response_valid.write(0);
  response.write(0);
  int count_in_addends = 0;
  int count_out_addends = 0;
  wait();

  int curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
  std::cout << "adder.cpp is before while loop at cycle " << curr_cycle << std::endl;
  // Always @ positive edge of the clock
  while (true) {
    curr_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
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
      std::cout << module_name << ": Got Transaction " << count_in_addends << " on cycle " << curr_cycle << " (user = "
                << axis_adder_interface.tuser.read().to_uint64() << ") (addend = "
                << axis_adder_interface.tdata.read().to_uint64() << ")!"
                << std::endl;
      count_in_addends++;
        
        //adder_tdata_tlast_fifo.push(std::make_tuple(axis_adder_interface.tdata.read(), axis_adder_interface.tlast.read()));
    }

    /*if (adder_tdata_tlast_fifo.size() > 0) { //fifo not empty
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
    } */

    //sent to portal module
    if (axis_adder_master_interface.tvalid.read() && axis_adder_master_interface.tready.read()) {
        std::cout << "Sent the " << count_out_addends << "th addend over NoC to portal module on cycle " << curr_cycle << std::endl;
        //adder_tdata_tlast_fifo.pop();
        count_out_addends++;
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

  port_name = module_name + ".axis_adder_master_interface";
  RegisterAxisMasterPort(port_name, &axis_adder_master_interface, DATAW, 0);
}