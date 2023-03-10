#include <mem_controller_test.hpp>

mem_controller_test::mem_controller_test(const sc_module_name& name, 
    unsigned int num_cmds, unsigned int test_mode, unsigned int burst_size, 
    unsigned int num_channels, unsigned int mem_capacity_mb, 
    unsigned int num_used_channels, unsigned int addressable_word_size_bytes,
    double clk_period) : sc_module(name) {

  tx_interface.init(num_channels);

  _burst_size = burst_size;
  _num_channels = num_channels;
  _num_used_channels = num_used_channels;
  _mem_capacity = mem_capacity_mb;
  _bl_bits = (unsigned int) log2(addressable_word_size_bytes);
  _clk_period = clk_period;

  if (test_mode == 0) {
    _num_write_cmds = 0;
    _num_read_cmds = num_cmds;
  } else if (test_mode == 1) {
    _num_write_cmds = num_cmds;
    _num_read_cmds = 0;
  } else {
    _num_write_cmds = num_cmds / 2;
    _num_read_cmds = num_cmds / 2;
  }

  _w_count.resize(_num_used_channels, 0); 
  _aw_count.resize(_num_used_channels, 0); 
  _b_count.resize(_num_used_channels, 0); 
  _r_count.resize(_num_used_channels, 0); 
  _ar_count.resize(_num_used_channels, 0); 

  _waddr.resize(_num_used_channels);
  _raddr.resize(_num_used_channels);
  _wdata.resize(_num_used_channels);
  _read_issue_cycles.resize(_num_used_channels);
  _read_latency.resize(_num_used_channels);

  r_done = false;
  b_done = false;

  uint64_t max_offset_address = (_mem_capacity / _num_channels) * 1024 * 1024;
  unsigned int max_offset_address_bits = (unsigned int) log2(max_offset_address);
  uint64_t max_data = 4096;

  for (unsigned int ch_id = 0; ch_id < _num_used_channels; ch_id++) {
    for (unsigned int cmd_id = 0; cmd_id < _num_write_cmds / _burst_size; cmd_id++) {
      uint64_t offset_addr = (uint64_t) std::rand() % max_offset_address;
      while (offset_addr >= (max_offset_address - (_burst_size * (1 << _bl_bits))))
        offset_addr = (uint64_t) std::rand() % max_offset_address;
      uint64_t addr = ((uint64_t) ch_id) << max_offset_address_bits;
      addr += offset_addr;
      addr = (addr >> _bl_bits) << _bl_bits;
      _waddr[ch_id].push_back(addr);
    }

    for (unsigned int cmd_id = 0; cmd_id < _num_write_cmds; cmd_id++) {
      uint64_t data = (uint64_t) std::rand() % max_data;
      _wdata[ch_id].push_back(data);
    }

    for (unsigned int cmd_id = 0; cmd_id < _num_read_cmds / _burst_size; cmd_id++) {
      uint64_t offset_addr = (uint64_t) std::rand() % max_offset_address;
      while (offset_addr >= (max_offset_address - (_burst_size * (1 << _bl_bits))))
        offset_addr = (uint64_t) std::rand() % max_offset_address;
      uint64_t addr = ((uint64_t) ch_id) << max_offset_address_bits;
      addr += offset_addr;
      addr = (addr >> _bl_bits) << _bl_bits;
      _raddr[ch_id].push_back(addr);
    }
  }
  
  SC_METHOD(assign);
  sensitive << rst;
  SC_CTHREAD(aw_source, clk.pos());
  SC_CTHREAD(w_source, clk.pos());
  SC_CTHREAD(ar_source, clk.pos());
  SC_CTHREAD(b_sink, clk.pos());
  SC_CTHREAD(r_sink, clk.pos());
}

mem_controller_test::~mem_controller_test() {

}

void mem_controller_test::aw_source() {
  // Reset
  rst.write(true);
  for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
    tx_interface[ch_id].awvalid.write(false);
  }
  wait();
  rst.write(false);
  wait();

  bool aw_done = (_num_write_cmds == 0);

  while (!aw_done) {
    for (unsigned int ch_id = 0; ch_id < _num_used_channels; ch_id++) {
      if (tx_interface[ch_id].awvalid.read() && tx_interface[ch_id].awready.read()) {
        std::cout << "[" << GetSimulationCycle(_clk_period) << "] ";
        std::cout << "Channel " << ch_id << " accepted AW request ";
        std::cout << "(" << _aw_count[ch_id] << ", ";
        std::cout << _waddr[ch_id][_aw_count[ch_id]]<< ")" << std::endl;
        _aw_count[ch_id]++;
      }

      if (_aw_count[ch_id] < (_num_write_cmds/_burst_size)) {
        tx_interface[ch_id].awvalid.write(true);
        tx_interface[ch_id].awaddr.write(_waddr[ch_id][_aw_count[ch_id]]);
        tx_interface[ch_id].awlen.write(_burst_size);
      } else {
        tx_interface[ch_id].awvalid.write(false);
        tx_interface[ch_id].awaddr.write(0);
        tx_interface[ch_id].awlen.write(0);
      }
    }
    wait();
    aw_done = true;
    for (unsigned int ch_id = 0; ch_id < _num_used_channels; ch_id++) {
      aw_done = aw_done && (_aw_count[ch_id] == (_num_write_cmds/_burst_size));
    }
  }
  for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
    tx_interface[ch_id].awvalid.write(false);
  }
  if (_num_write_cmds != 0) {
    std::cout << "[" << GetSimulationCycle(_clk_period) << "] ";
    std::cout << "All AW commands sent!" << std::endl;
  }
  wait();
}

void mem_controller_test::w_source() {
  // Reset
  for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
    tx_interface[ch_id].wvalid.write(false);
  }
  wait();
  wait();

  bool w_done = (_num_write_cmds == 0);

  while (!w_done) {
    for (unsigned int ch_id = 0; ch_id < _num_used_channels; ch_id++) {
      if (tx_interface[ch_id].wvalid.read() && tx_interface[ch_id].wready.read()) {
        std::cout << "[" << GetSimulationCycle(_clk_period) << "] ";
        std::cout << "Channel " << ch_id << " accepted W request ";
        std::cout << "(" << _w_count[ch_id] << ", ";
        std::cout << _wdata[ch_id][_w_count[ch_id]].to_uint() << ")" << std::endl;
        _w_count[ch_id]++;
      }

      if (_w_count[ch_id] < _num_write_cmds) {
        tx_interface[ch_id].wvalid.write(true);
        tx_interface[ch_id].wdata.write(_wdata[ch_id][_w_count[ch_id]]);
        tx_interface[ch_id].wuser.write(0);
        tx_interface[ch_id].wlast.write((_w_count[ch_id] % _burst_size) == (_burst_size - 1));
      } else {
        tx_interface[ch_id].wvalid.write(false);
        tx_interface[ch_id].wdata.write(0);
        tx_interface[ch_id].wuser.write(0);
        tx_interface[ch_id].wlast.write(false);
      }
    }
    wait();
    w_done = true;
    for (unsigned int ch_id = 0; ch_id < _num_used_channels; ch_id++) {
      w_done = w_done && (_w_count[ch_id] == _num_write_cmds);
    }
  }
  for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
    tx_interface[ch_id].wvalid.write(false);
  }
  if (_num_write_cmds != 0) {
    std::cout << "[" << GetSimulationCycle(_clk_period) << "] ";
    std::cout << "All W commands sent!" << std::endl;
  }
  wait();
}

void mem_controller_test::ar_source() {
  // Reset
  for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
    tx_interface[ch_id].arvalid.write(false);
  }
  wait();
  wait();

  bool ar_done = (_num_read_cmds == 0);

  while (!ar_done) {
    for (unsigned int ch_id = 0; ch_id < _num_used_channels; ch_id++) {
      if (tx_interface[ch_id].arvalid.read() && tx_interface[ch_id].arready.read()) {
        std::cout << "[" << GetSimulationCycle(_clk_period) << "] ";
        std::cout << "Channel " << ch_id << " accepted AR request ";
        std::cout << "(" << _ar_count[ch_id] << ", ";
        std::cout << _raddr[ch_id][_ar_count[ch_id]]<< ")" << std::endl;
        _ar_count[ch_id]++;
        _read_issue_cycles[ch_id].push_back(GetSimulationCycle(_clk_period));
      }

      if (_ar_count[ch_id] < (_num_read_cmds/_burst_size)) {
        tx_interface[ch_id].arvalid.write(true);
        tx_interface[ch_id].araddr.write(_raddr[ch_id][_ar_count[ch_id]]);
        tx_interface[ch_id].arlen.write(_burst_size);
      } else {
        tx_interface[ch_id].arvalid.write(false);
        tx_interface[ch_id].araddr.write(0);
        tx_interface[ch_id].arlen.write(0);
      }
    }
    wait();
    ar_done = true;
    for (unsigned int ch_id = 0; ch_id < _num_used_channels; ch_id++) {
      ar_done = ar_done && (_ar_count[ch_id] == (_num_read_cmds/_burst_size));
    }
  }
  for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
    tx_interface[ch_id].arvalid.write(false);
  }
  if (_num_read_cmds != 0) {
    std::cout << "[" << GetSimulationCycle(_clk_period) << "] ";
    std::cout << "All AR commands sent!" << std::endl;
  }
  wait();
}

void mem_controller_test::r_sink() {
  r_done = false;

  while (!r_done) {
    for (unsigned int ch_id = 0; ch_id < _num_used_channels; ch_id++) {
      if (tx_interface[ch_id].rready.read() && tx_interface[ch_id].rvalid.read()) {
        std::cout << "[" << GetSimulationCycle(_clk_period) << "] ";
        std::cout << "Channel " << ch_id << " received R response " << _r_count[ch_id] << std::endl;
        if (_r_count[ch_id] % _burst_size == 0)
          _read_latency[ch_id].push_back(GetSimulationCycle(_clk_period) - _read_issue_cycles[ch_id][_r_count[ch_id]/_burst_size]);
        _r_count[ch_id]++;
      }
    }
    wait();
    r_done = true;
    for (unsigned int ch_id = 0; ch_id < _num_used_channels; ch_id++) {
      r_done = r_done && (_r_count[ch_id] == _num_read_cmds);
    }
  }
  if (_num_read_cmds != 0) {
    std::cout << "[" << GetSimulationCycle(_clk_period) << "] ";
    std::cout << "All R responses received!" << std::endl;
  }

  while(!(r_done && b_done)) wait();

  std::cout << "**********************" << std::endl;
  std::cout << "Average read latencies" << std::endl;
  std::cout << "**********************" << std::endl;
  for (unsigned int ch_id = 0; ch_id < _num_used_channels; ch_id++) {
    double sum = 0;
    for (unsigned int cmd_id = 0; cmd_id < _read_latency[ch_id].size(); cmd_id++) {
      sum += _read_latency[ch_id][cmd_id];
    }
    sum /= _read_latency[ch_id].size();
    std::cout << "Channel " << ch_id << ": " << int(sum) << " cycles" << std::endl;
  }
  sc_stop();
}

void mem_controller_test::b_sink() {
  b_done = false;

  while (!b_done) {
    for (unsigned int ch_id = 0; ch_id < _num_used_channels; ch_id++) {
      if (tx_interface[ch_id].bready.read() && tx_interface[ch_id].bvalid.read()) {
        std::cout << "[" << GetSimulationCycle(_clk_period) << "] ";
        std::cout << "Channel " << ch_id << " received B response " << _b_count[ch_id] << std::endl;
        _b_count[ch_id]++;
      }
    }
    wait();
    b_done = true;
    for (unsigned int ch_id = 0; ch_id < _num_used_channels; ch_id++) {
      b_done = b_done && (_b_count[ch_id] == (_num_write_cmds/_burst_size));
    }
  }
  if (_num_write_cmds != 0) {
    std::cout << "[" << GetSimulationCycle(_clk_period) << "] ";
    std::cout << "All B responses received!" << std::endl;
  }
}

void mem_controller_test::assign() {
  for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
    tx_interface[ch_id].bready.write(true);
    tx_interface[ch_id].rready.write(true);
  }
}

mem_controller_system::mem_controller_system(const sc_module_name& name) : sc_module(name) {
  double clk_period = 2.0;
  double mem_clk_period = 1.0;
  unsigned int total_cmds = 10 * 1024;
  unsigned int mode = 2; // 0: Read, 1: Write, 2: Read/Write
  unsigned int burst_size = 64;
  unsigned int num_used_channels = 8;
  unsigned int queue_sizes = 64;
  
  clk_sig = new sc_clock("clk0", clk_period, SC_NS);
  mem_clk_sig = new sc_clock("mem_clk", mem_clk_period, SC_NS);

  dut_inst = new mem_controller("mem_controller", 
    "/media/andrew/DATA/research/rad-flow/rad-sim/sim/dram/DRAMsim3/configs/HBM2_8Gb_x128.ini",
    "/media/andrew/DATA/research/rad-flow/rad-sim/build/",
    512, clk_period, queue_sizes, queue_sizes, true);
  dut_inst->clk(*clk_sig);
  dut_inst->mem_clk(*mem_clk_sig);
  dut_inst->rst(rst_sig);

  unsigned int num_channels = dut_inst->GetNumChannels();
  test_to_dut.init(num_channels);

  test_inst = new mem_controller_test("mem_controller_test", total_cmds, mode, 
    burst_size, num_channels, dut_inst->GetMemCapacity(), num_used_channels,
    dut_inst->GetAddressableWordSize(), clk_period);
  test_inst->clk(*clk_sig);
  test_inst->rst(rst_sig);

  for (unsigned int ch_id = 0; ch_id < num_channels; ch_id++) {
    test_to_dut[ch_id].Connect(test_inst->tx_interface[ch_id], dut_inst->mem_channels[ch_id]);
  }
}

mem_controller_system::~mem_controller_system() {
  delete clk_sig;
  delete dut_inst;
  delete test_inst;
}