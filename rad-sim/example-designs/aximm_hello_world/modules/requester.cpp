#include <requester.hpp>

requester::requester(const sc_module_name &name, unsigned int fifo_depth)
    : radsim_module(name) {

  req_fifo_depth = fifo_depth;

  // Combinational logic and its sensitivity list
  SC_METHOD(Assign);
  sensitive << rst << req_fifo_full;
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

requester::~requester() {}

void requester::Assign() {
  if (rst) {
    req_ready.write(false);
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

void requester::Tick() {
  // Reset logic
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
  wait();

  std::string port_name = module_name + ".aximm_req_interface";

  // Always @ positive edge of the clock
  while (true) {
    // Interface with testbench driver
    if (req_ready.read() && req_valid.read()) {
      req_addr_fifo.push(req_addr);
      req_wdata_fifo.push(req_wdata);
      req_type_fifo.push(req_type);
    }
    req_fifo_full.write(req_addr_fifo.size() >= req_fifo_depth);

    // Sending transactions to AXI-MM NoC
    sc_bv<DATAW> wdata = req_wdata_fifo.front();
    bool type = req_type_fifo.front();
    uint64_t addr = req_addr_fifo.front();

    if (type == 0) {
      aximm_req_interface.araddr.write(addr);
      aximm_req_interface.arid.write(id_counter.read());
      aximm_req_interface.arlen.write(0);
      aximm_req_interface.arburst.write(0);
      aximm_req_interface.arsize.write(DATAW / 8);
      aximm_req_interface.aruser.write(
          radsim_design.GetPortBaseAddress(port_name));
      aximm_req_interface.arvalid.write(true);

      aximm_req_interface.awvalid.write(false);
      aximm_req_interface.wvalid.write(false);
    } else {
      aximm_req_interface.awaddr.write(addr);
      aximm_req_interface.awid.write(id_counter.read());
      aximm_req_interface.awlen.write(0);
      aximm_req_interface.awburst.write(0);
      aximm_req_interface.awsize.write(DATAW / 8);
      aximm_req_interface.awuser.write(
          radsim_design.GetPortBaseAddress(port_name));
      aximm_req_interface.awvalid.write(!aw_accepted.read());

      aximm_req_interface.wid.write(id_counter.read());
      aximm_req_interface.wdata.write(wdata);
      aximm_req_interface.wlast.write(true);
      aximm_req_interface.wuser.write(
          radsim_design.GetPortBaseAddress(port_name));
      aximm_req_interface.wvalid.write(true);

      aximm_req_interface.arvalid.write(false);
    }
    if (aximm_req_interface.awvalid.read() &&
        aximm_req_interface.awready.read()) {
      aw_accepted.write(true);
      id_counter.write(id_counter.read() + 1);
    } else if (aximm_req_interface.wvalid.read() &&
               aximm_req_interface.wready.read()) {
      aw_accepted.write(false);
      req_addr_fifo.pop();
      req_type_fifo.pop();
      req_wdata_fifo.pop();
      id_counter.write(id_counter.read() + 1);
    } else if (aximm_req_interface.arvalid.read() &&
               aximm_req_interface.arready.read()) {
      req_addr_fifo.pop();
      req_type_fifo.pop();
      req_wdata_fifo.pop();
      id_counter.write(id_counter.read() + 1);
    }

    // Receiving transactions from AXI-MM NoC
    if (aximm_req_interface.rvalid.read() &&
        aximm_req_interface.rready.read()) {
      std::cout << "Received READ RESPONSE: " << std::endl;
      std::cout << "\trid: " << aximm_req_interface.rid.read() << std::endl;
      std::cout << "\truser: " << aximm_req_interface.ruser.read() << std::endl;
      std::cout << "\trdata: " << aximm_req_interface.rdata.read() << std::endl;
    } else if (aximm_req_interface.bvalid.read() &&
               aximm_req_interface.bready.read()) {
      std::cout << "Received WRITE RESPONSE: " << std::endl;
      std::cout << "\tbid: " << aximm_req_interface.bid.read() << std::endl;
      std::cout << "\tbuser: " << aximm_req_interface.buser.read() << std::endl;
    }
    wait();
  }
}

void requester::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;

  port_name = module_name + ".aximm_req_interface";
  RegisterAximmMasterPort(port_name, &aximm_req_interface, DATAW);
}
