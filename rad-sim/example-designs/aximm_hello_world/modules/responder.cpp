#include <responder.hpp>

responder::responder(const sc_module_name &name, unsigned int fifo_depth)
    : radsim_module(name) {

  req_fifo_depth = fifo_depth;
  resp_fifo_depth = fifo_depth;

  // Combinational logic and its sensitivity list
  SC_METHOD(Assign);
  sensitive << rst << write_req_fifo_full << write_data_fifo_full
            << read_req_fifo_full << resp_fifo_empty << resp_fifo_full;
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

responder::~responder() {}

void responder::Assign() {
  if (rst) {
    aximm_resp_interface.arready.write(false);
    aximm_resp_interface.awready.write(false);
    aximm_resp_interface.wready.write(false);
  } else {
    // Ready to accept transaction if the corresponding request FIFO and the
    // outgoing response FIFO are not full
    aximm_resp_interface.arready.write(!read_req_fifo_full.read() &&
                                       !resp_fifo_full.read());
    aximm_resp_interface.awready.write(!write_req_fifo_full.read() &&
                                       !resp_fifo_full.read());
    aximm_resp_interface.wready.write(!write_data_fifo_full.read() &&
                                      !resp_fifo_full.read());
  }
}

void responder::Tick() {
  // Reset logic
  aximm_resp_interface.bvalid.write(false);
  aximm_resp_interface.rvalid.write(false);
  write_req_fifo_full.write(false);
  write_data_fifo_full.write(false);
  read_req_fifo_full.write(false);
  resp_fifo_empty.write(true);
  resp_fifo_full.write(false);
  id_counter.write(0);
  wait();

  std::string port_name = module_name + ".aximm_resp_interface";
  uint64_t port_base_addr = radsim_design.GetPortBaseAddress(port_name);

  // Always @ positive edge of the clock
  while (true) {
    // Receiving transaction from AXI-MM interface
    if (aximm_resp_interface.arvalid.read() &&
        aximm_resp_interface.arready.read()) {
      req_araddr_fifo.push(aximm_resp_interface.araddr.read().to_uint64());
      req_raddr_fifo.push(aximm_resp_interface.aruser.read().to_uint64());
    } else if (aximm_resp_interface.awvalid.read() &&
               aximm_resp_interface.awready.read()) {
      req_waddr_fifo.push(aximm_resp_interface.awaddr.read().to_uint64());
      req_baddr_fifo.push(aximm_resp_interface.awuser.read().to_uint64());
    } else if (aximm_resp_interface.wvalid.read() &&
               aximm_resp_interface.wready.read()) {
      req_wdata_fifo.push(aximm_resp_interface.wdata.read());
    }

    // Executing a request
    if (!req_raddr_fifo.empty() && !req_araddr_fifo.empty()) {
      uint64_t araddr = req_araddr_fifo.front();
      uint64_t raddr = req_raddr_fifo.front();
      req_araddr_fifo.pop();
      req_raddr_fifo.pop();
      sc_bv<DATAW> rdata;
      if (storage.find(araddr) == storage.end()) {
        rdata = 0;
      } else {
        rdata = storage[araddr];
      }
      resp_data_fifo.push(rdata);
      resp_addr_fifo.push(raddr);
      resp_type_fifo.push(0);
    } else if (!req_waddr_fifo.empty() && !req_wdata_fifo.empty() &&
               !req_baddr_fifo.empty()) {
      uint64_t waddr = req_waddr_fifo.front();
      uint64_t baddr = req_baddr_fifo.front();
      sc_bv<DATAW> wdata = req_wdata_fifo.front();
      req_waddr_fifo.pop();
      req_baddr_fifo.pop();
      req_wdata_fifo.pop();
      storage[waddr] = wdata;
      resp_data_fifo.push(wdata);
      resp_addr_fifo.push(baddr);
      resp_type_fifo.push(1);
    }

    // Sending transaction over AXI-MM interface
    if (!resp_data_fifo.empty()) {
      uint64_t addr = resp_addr_fifo.front();
      sc_bv<DATAW> data = resp_data_fifo.front();
      bool type = resp_addr_fifo.front();
      if (type == 0) {
        aximm_resp_interface.rid.write(id_counter.read());
        aximm_resp_interface.rdata.write(data);
        aximm_resp_interface.rlast.write(true);
        aximm_resp_interface.ruser.write(
            radsim_design.GetPortBaseAddress(port_name));
        aximm_resp_interface.rresp.write(0);
        aximm_resp_interface.rvalid.write(true);
        aximm_resp_interface.bvalid.write(false);
      } else {
        aximm_resp_interface.bid.write(id_counter.read());
        aximm_resp_interface.buser.write(
            radsim_design.GetPortBaseAddress(port_name));
        aximm_resp_interface.bresp.write(0);
        aximm_resp_interface.bvalid.write(true);
        aximm_resp_interface.rvalid.write(false);
      }
    } else {
      aximm_resp_interface.rvalid.write(false);
      aximm_resp_interface.bvalid.write(false);
    }

    // Pop response from FIFOs if accepted by the AXI-MM interface
    if (aximm_resp_interface.rvalid.read() &&
        aximm_resp_interface.rready.read()) {
      resp_addr_fifo.pop();
      resp_data_fifo.pop();
      resp_type_fifo.pop();
      id_counter.write(id_counter.read() + 1);
    } else if (aximm_resp_interface.bvalid.read() &&
               aximm_resp_interface.bready.read()) {
      resp_addr_fifo.pop();
      resp_data_fifo.pop();
      resp_type_fifo.pop();
      id_counter.write(id_counter.read() + 1);
    }

    // Set FIFO full/empty signals
    write_req_fifo_full.write(req_waddr_fifo.size() >= req_fifo_depth);
    write_data_fifo_full.write(req_wdata_fifo.size() >= req_fifo_depth);
    read_req_fifo_full.write(req_araddr_fifo.size() >= req_fifo_depth);
    resp_fifo_empty.write(resp_data_fifo.empty());
    resp_fifo_full.write(resp_data_fifo.size() >= resp_fifo_depth);
    wait();
  }
}

void responder::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  _num_noc_aximm_slave_ports = 0;
  _num_noc_aximm_master_ports = 0;

  port_name = module_name + ".aximm_resp_interface";
  RegisterAximmSlavePort(port_name, &aximm_resp_interface, DATAW);
}