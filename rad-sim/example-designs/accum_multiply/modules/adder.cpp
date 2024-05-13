#include <adder.hpp>

//TODO
/*
constructor: define depth and initialize counter to 0. Assign only sensitive on FIFO variables

assign: we probably don't need any assign signals, since we only write from this using axis. Maybe use this for fifo ren and wen

tick: 
  reset logic: 
    all signals to NOT READY (axis)
    clear all FIFO content, 
    reset count, 
    clear all registers
  check if input ready and input valid, push to ififo
  if counter is 4, and ofifo isn't full, push to ofifo.
  check if counter is NOT 4, and input NOT empty, pop from ififo (ren) and into registers (which are shifted), increment counter by 1
  axis interface write OFIFO content and ren ofifo. (to pop)

some specific coding considerations:
  ren for ififo should be in an if else statement, default ren to false. 
  if(count is right and i not empty){
    read from ififo input signal
    ren=true // this removes rdata on next clock edge, so next cycle if we should read it will be new data. 
  }else{
    ren=false
  }
*/

adder::adder(const sc_module_name &name, unsigned int ififo_depth, unsigned int ofifo_depth)
    : RADSimModule(name) {
  // Define key constants
  this->ififo_depth = ififo_depth;
  this->ofifo_depth = ofifo_depth;

  // Initialize value to 0
  num_values_received = 0;

  // Initialize FIFO modules
  char fifo_name[25];
  std::string fifo_name_str;
  fifo_name_str = "adder" + std::to_string(mvm_id) + "_ififo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  ififo = new fifo<int16_t>(fifo_name, ififo_depth, 16, ofifo_depth-1, 0); // width is 16 for int16, almost_full is 1 less
  ififo->clk(clk);
  ififo->rst(rst);
  ififo->wen(ififo_wen_signal);
  ififo->ren(ififo_ren_signal);
  ififo->wdata(ififo_wdata_signal);
  ififo->full(ififo_full_signal);
  ififo->almost_full(ififo_almost_full_signal);
  ififo->empty(ififo_empty_signal);
  ififo->almost_empty(ififo_almost_empty_signal);
  ififo->rdata(ififo_rdata_signal);

  fifo_name_str = "adder" + std::to_string(mvm_id) + "_ofifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  ofifo = new fifo<int16_t>(fifo_name, ofifo_depth, 16, ofifo_depth-1, 0); // width is 16 for int16, almost_full is 1 less
  ofifo->clk(clk);
  ofifo->rst(rst);
  ofifo->wen(ofifo_wen_signal);
  ofifo->ren(ofifo_ren_signal);
  ofifo->wdata(ofifo_wdata_signal);
  ofifo->full(ofifo_full_signal);
  ofifo->almost_full(ofifo_almost_full_signal);
  ofifo->empty(ofifo_empty_signal);
  ofifo->almost_empty(ofifo_almost_empty_signal);
  ofifo->rdata(ofifo_rdata_signal);

  // Combinational logic and its sensitivity list TODO
  SC_METHOD(Assign);
  sensitive << rst << req_fifo_full;
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

adder::~adder() {}

void adder::Assign() {
  // TODO, might use for axis interface
  if (rst) {
    req_ready.write(true);
    aximm_req_interface.bready.write(false);
    aximm_req_interface.rready.write(false);
  } else {
    // Ready to accept new request from driver testbench as long as the request
    // FIFO is not full
    req_ready.write(!req_fifo_full.read());
    // Always ready to accept read/write response from the AXI-MM NoC interface
    aximm_req_interface.bready.write(true);
    aximm_req_interface.rready.write(true);
  }
}

void adder::Tick() {
  // Reset logic
  // Set axis signals to default not ready
  // Clear FIFO content, set signals to not ready
  // Clear Registers
  // Reset Count

  aximm_req_interface.arvalid.write(false);
  aximm_req_interface.awvalid.write(false);
  aximm_req_interface.wvalid.write(false);
  while (!req_addr_fifo.empty()) {
    req_addr_fifo.pop();
    req_type_fifo.pop();
    req_wdata_fifo.pop();
  }
  req_fifo_full.write(false);
  aw_accepted.write(false);
  id_counter.write(0);
  received_responses.write(0);
  wait();

  std::string port_name = module_name + ".aximm_req_interface";

  // Always @ positive edge of the clock
  while (true) {
    // Interface with testbench driver
    if (req_ready.read() && req_valid.read()) {
      req_addr_fifo.push(req_addr);
      req_wdata_fifo.push(req_wdata);
      req_type_fifo.push(req_type);
      std::cout << module_name << ": Pushed request to FIFO" << std::endl;
    }
    req_fifo_full.write(req_addr_fifo.size() >= req_fifo_depth);

    // Sending transactions to AXI-MM NoC
    if (!req_wdata_fifo.empty()) {
      sc_bv<DATAW> wdata = req_wdata_fifo.front();
      bool type = req_type_fifo.front();
      std::string dst_port_name = "multiplier_inst.axis_multiplier_interface";
      uint64_t dst_addr = radsim_design.GetPortBaseAddress(dst_port_name) +
                          req_addr_fifo.front();
      std::string src_port_name = "adder_inst.axis_adder_interface";
      uint64_t src_addr = radsim_design.GetPortBaseAddress(src_port_name);

      if (type == 0) {
        aximm_req_interface.araddr.write(dst_addr);
        aximm_req_interface.arid.write(id_counter.read());
        aximm_req_interface.arlen.write(0);
        aximm_req_interface.arburst.write(0);
        aximm_req_interface.arsize.write(DATAW / 8);
        aximm_req_interface.aruser.write(src_addr);
        aximm_req_interface.arvalid.write(true);

        aximm_req_interface.awvalid.write(false);
        aximm_req_interface.wvalid.write(false);
      } else {
        aximm_req_interface.awaddr.write(dst_addr);
        aximm_req_interface.awid.write(id_counter.read());
        aximm_req_interface.awlen.write(0);
        aximm_req_interface.awburst.write(0);
        aximm_req_interface.awsize.write(DATAW / 8);
        aximm_req_interface.awuser.write(src_addr);
        aximm_req_interface.awvalid.write(!aw_accepted.read());

        aximm_req_interface.wid.write(id_counter.read());
        aximm_req_interface.wdata.write(wdata);
        aximm_req_interface.wlast.write(true);
        aximm_req_interface.wuser.write(src_addr);
        aximm_req_interface.wvalid.write(true);

        aximm_req_interface.arvalid.write(false);
      }
    } else {
      aximm_req_interface.arvalid.write(false);
      aximm_req_interface.awvalid.write(false);
      aximm_req_interface.wvalid.write(false);
    }

    if (aximm_req_interface.awvalid.read() &&
        aximm_req_interface.awready.read()) {
      aw_accepted.write(true);
      id_counter.write(id_counter.read() + 1);
      std::cout << module_name << ": Sent AW Transaction!" << std::endl;
    } else if (aximm_req_interface.wvalid.read() &&
               aximm_req_interface.wready.read()) {
      aw_accepted.write(false);
      req_addr_fifo.pop();
      req_type_fifo.pop();
      req_wdata_fifo.pop();
      id_counter.write(id_counter.read() + 1);
      std::cout << module_name << ": Sent W Transaction!" << std::endl;
    } else if (aximm_req_interface.arvalid.read() &&
               aximm_req_interface.arready.read()) {
      req_addr_fifo.pop();
      req_type_fifo.pop();
      req_wdata_fifo.pop();
      id_counter.write(id_counter.read() + 1);
      std::cout << module_name << ": Sent AR Transaction!" << std::endl;
    }

    // Receiving transactions from AXI-MM NoC
    if (aximm_req_interface.rvalid.read() &&
        aximm_req_interface.rready.read()) {
      std::cout << "Received READ RESPONSE: " << std::endl;
      std::cout << "\trid: " << aximm_req_interface.rid.read() << std::endl;
      std::cout << "\truser: " << aximm_req_interface.ruser.read() << std::endl;
      std::cout << "\trdata: " << aximm_req_interface.rdata.read() << std::endl;
      num_received_responses++;
    } else if (aximm_req_interface.bvalid.read() &&
               aximm_req_interface.bready.read()) {
      std::cout << "Received WRITE RESPONSE: " << std::endl;
      std::cout << "\tbid: " << aximm_req_interface.bid.read() << std::endl;
      std::cout << "\tbuser: " << aximm_req_interface.buser.read() << std::endl;
      num_received_responses++;
    }
    received_responses.write(num_received_responses);
    wait();
  }
}

void addder::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;

  port_name = module_name + ".aximm_req_interface";
  RegisterAximmMasterPort(port_name, &aximm_req_interface, DATAW);
}
