#include <mem_controller.hpp>

mem_controller::mem_controller(const sc_module_name& name, 
  const std::string& config_file, 
  const std::string& output_dir, unsigned int controller_channel_bitwidth, 
  double controller_clk_period, unsigned int input_queue_size, 
  unsigned int output_queue_size, bool read_before_write) 
    : sc_module(name) {
  
  //module_name = name;
  _dramsim = new dramsim3::MemorySystem(config_file, output_dir,
      std::bind(&mem_controller::MemReadCallback, this, std::placeholders::_1),
      std::bind(&mem_controller::MemWriteCallback, this, std::placeholders::_1));
  _num_channels = _dramsim->GetChannels();
  mem_channels.init(_num_channels);
  
  _memory_channel_bitwidth = _dramsim->GetBusBits();
  _controller_channel_bitwidth = controller_channel_bitwidth;
  _memory_clk_period_ns = _dramsim->GetTCK();
  _controller_clk_period_ns = controller_clk_period;
  double bitwidth_ratio = 1.0 * _controller_channel_bitwidth / _memory_channel_bitwidth;
  double clk_period_ratio = 2.0 * _controller_clk_period_ns / _memory_clk_period_ns;
  if (bitwidth_ratio != clk_period_ratio)
    sim_log.log(error, "Ratios of controller/memory bitwidths and clock periods do not match!", name);

  _read_before_write = read_before_write;
  _mem_contents.resize(_num_channels);
  _write_address_queue.resize(_num_channels);
  _write_data_queue.resize(_num_channels);
  _read_address_queue.resize(_num_channels);
  _outstanding_write_requests.resize(_num_channels);
  _outstanding_read_requests.resize(_num_channels);
  _output_read_responses.resize(_num_channels);
  _output_write_responses.resize(_num_channels);
  _write_address_queue_occupancy.init(_num_channels);
  _write_data_queue_occupancy.init(_num_channels);
  _read_address_queue_occupancy.init(_num_channels);
  _num_outstanding_write_requests.init(_num_channels);
  _num_outstanding_read_requests.init(_num_channels);
  _output_write_queue_occupancy.init(_num_channels);
  _output_read_queue_occupancy.init(_num_channels);
  _input_queue_size = input_queue_size;
  _output_queue_size = output_queue_size;

  _num_ranks = _dramsim->GetRanks();
  _num_bank_groups = _dramsim->GetBankGroups();
  _num_banks_per_group = _dramsim->GetBanksPerGroup();
  _num_rows = _dramsim->GetRows();
  _num_word_cols = 1 << ((int)log2(_dramsim->GetCols()) - (int)log2(_dramsim->GetBurstLength()));
  _total_num_addressable_words = _num_channels * _num_ranks * _num_bank_groups * _num_banks_per_group * _num_rows
                                  * _num_word_cols;
  _total_num_addressable_words_M = _total_num_addressable_words / 1024 / 1024;     
  _addressable_size_bytes = _dramsim->GetBusBits() * _dramsim->GetBurstLength() / 8;             
  _total_mem_capacity_megabytes = _total_num_addressable_words_M * _addressable_size_bytes;
  
  _field_widths["ch"] = (int)log2(_num_channels);
  _field_widths["ra"] = (int)log2(_num_ranks);
  _field_widths["bg"] = (int)log2(_num_bank_groups);
  _field_widths["ba"] = (int)log2(_num_banks_per_group);
  _field_widths["ro"] = (int)log2(_num_rows);
  _field_widths["co"] = (int)log2(_num_word_cols);
  _field_widths["bl"] = (int)log2(_dramsim->GetBurstLength() * _dramsim->GetBusBits() / 8);
  std::string address_mapping = _dramsim->GetAddressMapping();
  for (unsigned int i = 0; i < address_mapping.size(); i += 2) {
      std::string token = address_mapping.substr(i, 2);
      _mem_addr_fields.push_back(token);
  }

  SC_METHOD(Assign);
  sensitive << rst;
  for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
    sensitive << _write_address_queue_occupancy[ch_id] << _read_address_queue_occupancy[ch_id]
      << _write_data_queue_occupancy[ch_id]
      << _num_outstanding_write_requests[ch_id] << _num_outstanding_read_requests[ch_id]
      << _output_write_queue_occupancy[ch_id] << _output_read_queue_occupancy[ch_id];
  }
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
  SC_CTHREAD(MemTick, mem_clk.pos());
  reset_signal_is(rst, true);

  this->PrintMemParameters();
  //this->RegisterModuleInfo();
}

mem_controller::~mem_controller() {
  delete _dramsim;
}

unsigned int mem_controller::GetNumChannels() { return _num_channels; }

unsigned int mem_controller::GetMemCapacity() { return _total_mem_capacity_megabytes; }

unsigned int mem_controller::GetAddressableWordSize() { 
  return _addressable_size_bytes;
}

void mem_controller::MemReadCallback(uint64_t addr) {
  unsigned int ch_id = ChannelFromAddr(addr);
  assert(!_outstanding_read_requests[ch_id].empty());
  uint64_t resp_addr = _outstanding_read_requests[ch_id].front().first;
  bool last = _outstanding_read_requests[ch_id].front().second;
  sc_bv<AXIS_MAX_DATAW> data_word;
  if (_mem_contents[ch_id].find(addr) != _mem_contents[ch_id].end()) {
    data_word = _mem_contents[ch_id][addr];
  } else {
    data_word = 0;
  }
  _outstanding_read_requests[ch_id].pop();
  _output_read_responses[ch_id].push(std::make_tuple(resp_addr, data_word, last));
  return;
}

void mem_controller::MemWriteCallback(uint64_t addr) {
  unsigned int ch_id = ChannelFromAddr(addr);
  assert(!_outstanding_write_requests[ch_id].empty());
  uint64_t resp_addr = std::get<0>(_outstanding_write_requests[ch_id].front());
  sc_bv<AXIS_MAX_DATAW> data_word = std::get<1>(_outstanding_write_requests[ch_id].front());
  bool last = std::get<2>(_outstanding_write_requests[ch_id].front());
  _outstanding_write_requests[ch_id].pop();
  if (last) _output_write_responses[ch_id].push(resp_addr);
  _mem_contents[ch_id][addr] = data_word;
  return;
}

void mem_controller::PrintStats() {
  _dramsim->PrintStats();
}

void mem_controller::PrintMemParameters() {
  std::cout << "Memory Parameters: " << std::endl;
  std::cout << "\tNo. of Addressable Words = " << _total_num_addressable_words << std::endl;
  std::cout << "\tTotal Capacity = " << _total_mem_capacity_megabytes << " MB" << std::endl;
  std::cout << "\tNo. of Channels = " << _num_channels << std::endl;
  std::cout << "\tNo. of Ranks = " << _num_ranks << std::endl;
  std::cout << "\tNo. of Bank Groups = " << _num_bank_groups << std::endl;
  std::cout << "\tNo. of Banks Per Group = " << _num_banks_per_group << std::endl;
  std::cout << "\tNo. of Rows = " << _num_rows << std::endl;
  std::cout << "\tNo. of Word Columns = " << _num_word_cols << std::endl;
  std::cout << "\tAddress Field Widths" << std::endl;
  std::cout << "\t\tch = " << _field_widths["ch"] << std::endl;
  std::cout << "\t\tra = " << _field_widths["ra"] << std::endl;
  std::cout << "\t\tbg = " << _field_widths["bg"] << std::endl;
  std::cout << "\t\tba = " << _field_widths["ba"] << std::endl;
  std::cout << "\t\tro = " << _field_widths["ro"] << std::endl;
  std::cout << "\t\tco = " << _field_widths["co"] << std::endl;
  std::cout << "\t\tbl = " << _field_widths["bl"] << std::endl;
}

uint64_t mem_controller::AddressMapping(uint64_t addr) {
  uint64_t translated_addr = 0;
  uint64_t working_addr = addr;

  // Split Address field values
  std::unordered_map<std::string, unsigned int> field_values;
  field_values["bl"] = working_addr & ((1 << _field_widths["bl"]) - 1);
  working_addr >>= _field_widths["bl"];
  field_values["co"] = working_addr & ((1 << _field_widths["co"]) - 1);
  working_addr >>= _field_widths["co"];
  field_values["ba"] = working_addr & ((1 << _field_widths["ba"]) - 1);
  working_addr >>= _field_widths["ba"];
  field_values["ro"] = working_addr & ((1 << _field_widths["ro"]) - 1);
  working_addr >>= _field_widths["ro"];
  field_values["bg"] = working_addr & ((1 << _field_widths["bg"]) - 1);
  working_addr >>= _field_widths["bg"];
  field_values["ra"] = working_addr & ((1 << _field_widths["ra"]) - 1);
  working_addr >>= _field_widths["ra"];
  field_values["ch"] = working_addr & ((1 << _field_widths["ch"]) - 1);

  // Arrange address fields
  for (unsigned int f = 0; f < _mem_addr_fields.size(); f++) {
    std::string token = _mem_addr_fields[f];
    translated_addr <<= _field_widths[token];
    translated_addr += field_values[token];
  }
  translated_addr <<= _field_widths["bl"];
  translated_addr += field_values["bl"];
  return translated_addr;
}

unsigned int mem_controller::ChannelFromAddr(uint64_t addr){
  unsigned int skip_bits = _field_widths["bl"];
  for (int f = _mem_addr_fields.size()-1; f >= 0; f--) {
    std::string token = _mem_addr_fields[f];
    if (token != "ch") {
      skip_bits += _field_widths[token];
    } else {
      break;
    }
  }
  uint64_t shifted_addr = (addr >> skip_bits);
  uint64_t mask = ((1 << _field_widths["ch"]) - 1);
  unsigned int channel_id = shifted_addr & mask;
  return channel_id;
}

void mem_controller::Tick() {
  // Reset logic
  for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
    _mem_contents[ch_id].clear();
    while(!_write_address_queue[ch_id].empty()) _write_address_queue[ch_id].pop();
    while(!_write_data_queue[ch_id].empty()) _write_data_queue[ch_id].pop();
    while(!_read_address_queue[ch_id].empty()) _read_address_queue[ch_id].pop();
    while(!_outstanding_write_requests[ch_id].empty()) _outstanding_write_requests[ch_id].pop();
    while(!_outstanding_read_requests[ch_id].empty()) _outstanding_read_requests[ch_id].pop();
    while(!_output_read_responses[ch_id].empty()) _output_read_responses[ch_id].pop();
    while(!_output_write_responses[ch_id].empty()) _output_write_responses[ch_id].pop();
    _write_address_queue_occupancy[ch_id].write(0);
    _write_data_queue_occupancy[ch_id].write(0);
    _read_address_queue_occupancy[ch_id].write(0);
  }
  wait();

  while(true) {
    for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
      // Accepting write address requests
      if (mem_channels[ch_id].awvalid.read() && mem_channels[ch_id].awready.read()) {
        unsigned int burst_size = mem_channels[ch_id].awlen.read().to_uint();
        uint64_t base_addr = mem_channels[ch_id].awaddr.read().to_uint64();
        for (unsigned int  b = 0; b < burst_size; b++) {
          uint64_t translated_addr = this->AddressMapping(base_addr + (b * _addressable_size_bytes));
          _write_address_queue[ch_id].push(std::make_pair(translated_addr, b == burst_size-1));
        }
      } 
      _write_address_queue_occupancy[ch_id].write(_write_address_queue[ch_id].size());
      // Accepting write data requests
      if (mem_channels[ch_id].wvalid.read() && mem_channels[ch_id].wready.read()) {
        uint64_t resp_addr = mem_channels[ch_id].wuser.read().to_uint();
        sc_bv<AXI_MAX_DATAW> write_data = mem_channels[ch_id].wdata.read();
        _write_data_queue[ch_id].push(std::make_pair(resp_addr, write_data));
      }
      _write_data_queue_occupancy[ch_id].write(_write_data_queue[ch_id].size());
      // Accepting read address requests
      if (mem_channels[ch_id].arvalid.read() && mem_channels[ch_id].arready.read()) {
        uint64_t resp_addr = mem_channels[ch_id].aruser.read().to_uint64();
        unsigned int burst_size = mem_channels[ch_id].arlen.read().to_uint();
        uint64_t base_addr = mem_channels[ch_id].araddr.read().to_uint64();
        for (unsigned int  b = 0; b < burst_size; b++) {
          uint64_t translated_addr = this->AddressMapping(base_addr + (b * _addressable_size_bytes));
          _read_address_queue[ch_id].push(std::make_tuple(resp_addr, translated_addr, b == burst_size-1));
        }
      }
      _read_address_queue_occupancy[ch_id].write(_read_address_queue[ch_id].size());
      // Sending write responses
      if (mem_channels[ch_id].bvalid.read() && mem_channels[ch_id].bready.read()) {
        _output_write_responses[ch_id].pop();
      }
      // Sending read responses
      if (mem_channels[ch_id].rvalid.read() && mem_channels[ch_id].rready.read()) {
        _output_read_responses[ch_id].pop();
      }
    }
    wait();
  }
}

void mem_controller::MemTick() {
  for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
    _num_outstanding_write_requests[ch_id].write(0);
    _num_outstanding_read_requests[ch_id].write(0);
    _output_write_queue_occupancy[ch_id].write(0);
    _output_read_queue_occupancy[ch_id].write(0);
  }
  wait();

  while(true) {
    _dramsim->ClockTick();
    for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
      _output_write_queue_occupancy[ch_id].write(_output_write_responses[ch_id].size());
      _output_read_queue_occupancy[ch_id].write(_output_read_responses[ch_id].size());
    }
    if (_read_before_write) {
      for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
        // Issue read request to DRAMsim
        if (_read_address_queue[ch_id].size() > 0) {
          std::tuple<uint64_t, uint64_t, bool> read_request = _read_address_queue[ch_id].front();
          uint64_t resp_addr = std::get<0>(read_request);
          uint64_t read_addr = std::get<1>(read_request);
          bool last = std::get<2>(read_request);
          if (_dramsim->WillAcceptTransaction(read_addr, false)) {
            _dramsim->AddTransaction(read_addr, false);
            _read_address_queue[ch_id].pop();
            _outstanding_read_requests[ch_id].push(std::make_pair(resp_addr, last));
          }
        // Issue write request to DRAMsim
        } else if (_write_data_queue[ch_id].size() > 0) {
          uint64_t resp_addr = _write_data_queue[ch_id].front().first;
          uint64_t write_addr = _write_address_queue[ch_id].front().first;
          bool last = _write_address_queue[ch_id].front().second;
          sc_bv<AXI_MAX_DATAW> write_data = _write_data_queue[ch_id].front().second;
          if (_dramsim->WillAcceptTransaction(write_addr, true)) {
            _dramsim->AddTransaction(write_addr, true);
            _write_data_queue[ch_id].pop();
            _write_address_queue[ch_id].pop();
            _outstanding_write_requests[ch_id].push(std::make_tuple(resp_addr, write_data, last));
          }
        }
        // Adjust outstanding request counters
        _num_outstanding_write_requests[ch_id].write(_outstanding_write_requests[ch_id].size());
        _num_outstanding_read_requests[ch_id].write(_outstanding_read_requests[ch_id].size());
      }
      _read_before_write = !_read_before_write;
    } else {
      for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
        // Issue write request to DRAMsim
        if (_write_data_queue[ch_id].size() > 0) {
          uint64_t resp_addr = _write_data_queue[ch_id].front().first;
          uint64_t write_addr = _write_address_queue[ch_id].front().first;
          bool last = _write_address_queue[ch_id].front().second;
          sc_bv<AXI_MAX_DATAW> write_data = _write_data_queue[ch_id].front().second;
          if (_dramsim->WillAcceptTransaction(write_addr, true)) {
            _dramsim->AddTransaction(write_addr, true);
            _write_data_queue[ch_id].pop();
            _write_address_queue[ch_id].pop();
            _outstanding_write_requests[ch_id].push(std::make_tuple(resp_addr, write_data, last));
          }
        // Issue read request to DRAMsim
        } else if (_read_address_queue[ch_id].size() > 0) {
          std::tuple<uint64_t, uint64_t, bool> read_request = _read_address_queue[ch_id].front();
          uint64_t resp_addr = std::get<0>(read_request);
          uint64_t read_addr = std::get<1>(read_request);
          bool last = std::get<2>(read_request);
          if (_dramsim->WillAcceptTransaction(read_addr, false)) {
            _dramsim->AddTransaction(read_addr, false);
            _read_address_queue[ch_id].pop();
            _outstanding_read_requests[ch_id].push(std::make_pair(resp_addr, last));
          }
          // Adjust outstanding request counters
          _num_outstanding_write_requests[ch_id].write(_outstanding_write_requests[ch_id].size());
          _num_outstanding_read_requests[ch_id].write(_outstanding_read_requests[ch_id].size());
        }
      }
      _read_before_write = !_read_before_write;
    }
    wait();
  }
}

void mem_controller::Assign() {
  if (rst.read()) {
    for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
      mem_channels[ch_id].awready.write(false);
      mem_channels[ch_id].wready.write(false);
      mem_channels[ch_id].bvalid.write(false);
      mem_channels[ch_id].arready.write(false);
      mem_channels[ch_id].rvalid.write(false);
    }
  } else {
    for (unsigned int ch_id = 0; ch_id < _num_channels; ch_id++) {
      bool read_input_queue_ok = _read_address_queue_occupancy[ch_id] < _input_queue_size;
      bool read_output_queue_ok = (_num_outstanding_read_requests[ch_id] + _output_read_queue_occupancy[ch_id]) < _output_queue_size;
      bool write_output_queue_ok = (_num_outstanding_write_requests[ch_id] + _output_write_queue_occupancy[ch_id]) < _output_queue_size;

      mem_channels[ch_id].awready.write(_write_address_queue_occupancy[ch_id] < _input_queue_size);
      mem_channels[ch_id].wready.write(_write_address_queue_occupancy[ch_id] > 0 
        && _write_data_queue_occupancy[ch_id] < _input_queue_size && write_output_queue_ok);
      mem_channels[ch_id].arready.write(read_input_queue_ok && read_output_queue_ok);

      mem_channels[ch_id].bvalid.write(_output_write_queue_occupancy[ch_id] > 0);
      if (_output_write_responses[ch_id].size() > 0) {  
        mem_channels[ch_id].buser.write(_output_write_responses[ch_id].front());
      }
      
      mem_channels[ch_id].rvalid.write(_output_read_queue_occupancy[ch_id] > 0);
      if (_output_read_responses[ch_id].size() > 0) {  
        mem_channels[ch_id].rdata.write(std::get<1>(_output_read_responses[ch_id].front()));
        mem_channels[ch_id].rlast.write(std::get<2>(_output_read_responses[ch_id].front()));
        mem_channels[ch_id].ruser.write(std::get<0>(_output_read_responses[ch_id].front()));
      }
    }
  }
}

/*void dram_axis::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_slave_ports = 0;
  _num_noc_master_ports = 0;

  port_name = module_name + ".tx_interface";
  RegisterMasterPort(port_name, &tx_interface, _mem.GetBusBits(), 0);

  port_name = module_name + ".rx_interface";
  RegisterSlavePort(port_name, &rx_interface, _mem.GetBusBits(), 0);
}*/