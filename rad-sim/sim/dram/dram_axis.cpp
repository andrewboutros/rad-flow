#include <dram_axis.hpp>

dram_axis::dram_axis(const sc_module_name& name, const std::string& config_file, 
  const std::string& output_dir) :
    sc_module(name),
    _mem(
      config_file, output_dir,
      std::bind(&dram_axis::MemReadCallback, this, std::placeholders::_1),
      std::bind(&dram_axis::MemWriteCallback, this, std::placeholders::_1)){
  
  //module_name = name;
  _rx_requests_addr = new sc_fifo<uint64_t>(4);
  _rx_requests_type = new sc_fifo<bool>(4);

  num_push.write(0);
  num_pop.write(0);
  rx_req_keep.write(false);

  SC_METHOD(Assign);
  sensitive << rst << num_push << num_pop << rx_interface.tvalid << tx_interface.tready
    << _rx_requests_addr->data_written_event();
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
  //this->RegisterModuleInfo();
}

dram_axis::~dram_axis() {
  delete _rx_requests_addr;
  delete _rx_requests_type;
}

void dram_axis::MemReadCallback(uint64_t addr) {
  sc_bv<AXIS_MAX_DATAW> data_word;
  if (_mem_contents.find(addr) != _mem_contents.end()) {
    data_word = _mem_contents[addr];
  } else {
    data_word = 0;
  }
  _read_resp_queue.push(std::make_pair(addr, data_word));
  num_push.write(num_push.read() + 1);
  return;
}

void dram_axis::MemWriteCallback(uint64_t addr) {
  (void) addr;
  return;
}

void dram_axis::PrintStats() {
  _mem.PrintStats();
}

void dram_axis::Tick() {
  // Reset logic
  while(!_read_resp_queue.empty()) _read_resp_queue.pop();
  num_push.write(0);
  num_pop.write(0);
  rx_req_keep.write(false);
  wait();

  while(true) {
    _mem.ClockTick();

    if (rx_interface.tready && rx_interface.tvalid) {
      uint64_t addr = rx_interface.tuser.read().range(63,0).to_uint();
      bool is_write = rx_interface.tuser.read().range(64,64).to_uint();
      _rx_requests_addr->write(addr);
      _rx_requests_type->write(is_write);
      _mem_contents[addr] = rx_interface.tdata.read();
    }

    if (rx_req_keep.read()) {
      if (_mem.WillAcceptTransaction(rx_req_addr, rx_req_type)) {
        _mem.AddTransaction(rx_req_addr, rx_req_type);
        if (_rx_requests_addr->num_available() > 0) {
          rx_req_keep.write(true);
          _rx_requests_addr->nb_read(rx_req_addr);
          _rx_requests_type->nb_read(rx_req_type);
        } else {
          rx_req_keep.write(false);
        }
      }
    } else if (_rx_requests_addr->num_available() > 0) {
      _rx_requests_addr->nb_read(rx_req_addr);
      _rx_requests_type->nb_read(rx_req_type);
      if (_mem.WillAcceptTransaction(rx_req_addr, rx_req_type)) {
        _mem.AddTransaction(rx_req_addr, rx_req_type);
      } else {
        rx_req_keep.write(true);
      }
    }

    if (tx_interface.tready && tx_interface.tvalid) {
      num_pop.write(num_pop.read() + 1);
      _read_resp_queue.pop();
    }

    wait();
  }
}

void dram_axis::Assign() {
  if (rst.read()) {
    rx_interface.tready.write(false);
    tx_interface.tdata.write(0);
    tx_interface.tvalid.write(false);
    tx_interface.tstrb.write((2 << AXIS_STRBW) - 1);
    tx_interface.tkeep.write((2 << AXIS_KEEPW) - 1);
    tx_interface.tlast.write(0);
    tx_interface.tuser.write(0);
    tx_interface.tdest.write(0);
  } else {
    if (num_push.read() > num_pop.read() && tx_interface.tready.read()) {
      std::pair<uint64_t, sc_bv<AXIS_MAX_DATAW>> read_data = _read_resp_queue.front();
      tx_interface.tdata.write(read_data.second);
      tx_interface.tvalid.write(true);
      tx_interface.tuser.write(read_data.first);
      tx_interface.tdest.write(0); // FIXME
    } else {
      tx_interface.tvalid.write(false);
    }

    rx_interface.tready.write(_rx_requests_addr->num_free() > 0);
  }
}

/*void dram_axis::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;

  port_name = module_name + ".tx_interface";
  RegisterAxisMasterPort(port_name, &tx_interface, _mem.GetBusBits(), 0);

  port_name = module_name + ".rx_interface";
  RegisterAxisSlavePort(port_name, &rx_interface, _mem.GetBusBits(), 0);
}*/