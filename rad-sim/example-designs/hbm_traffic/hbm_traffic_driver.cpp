#include <hbm_traffic_driver.hpp>

bool ParseInputs(std::vector<data_vector<uint64_t>> &lookup_indecies,
                 std::vector<data_vector<unsigned int>> &target_channels,
                 std::vector<data_vector<uint64_t>> &base_addresses,
                 std::string &io_filename) {
  std::ifstream io_file(io_filename);
  if (!io_file)
    return false;

  uint64_t num_indecies_per_input, index;
  std::string line;

  // Get number of indecies per input
  std::getline(io_file, line);
  std::stringstream header_stream(line);
  header_stream >> num_indecies_per_input;

  unsigned int line_num = 0;
  while (std::getline(io_file, line)) {
    std::stringstream line_stream(line);
    if (line_num % 3 == 0) {
      data_vector<uint64_t> dvector(num_indecies_per_input);
      for (unsigned int i = 0; i < num_indecies_per_input; i++) {
        line_stream >> index;
        dvector[i] = index;
      }
      lookup_indecies.push_back(dvector);
    } else if (line_num % 3 == 1) {
      data_vector<unsigned int> dvector(num_indecies_per_input);
      for (unsigned int i = 0; i < num_indecies_per_input; i++) {
        line_stream >> index;
        dvector[i] = index;
      }
      target_channels.push_back(dvector);
    } else {
      data_vector<uint64_t> dvector(num_indecies_per_input);
      for (unsigned int i = 0; i < num_indecies_per_input; i++) {
        line_stream >> index;
        dvector[i] = index;
      }
      base_addresses.push_back(dvector);
    }
    line_num++;
  }
  return true;
}


// Todo update verifcation s.t. instruction addresses are accounted for 
bool ParseOutputs(std::vector<uint64_t> &mem_req_outputs,
                    std::string &io_filename, unsigned int &num_outputs){
    std::ifstream io_file(io_filename);
    if (!io_file)
        return false;
    uint64_t inst_addr;
    uint64_t ch_idx;
    uint64_t data_addr;
    uint64_t rd_data;

    std::string line;
    std::getline(io_file, line);
    std::stringstream header_stream(line);
    header_stream >> num_outputs;

    while (std::getline(io_file, line)) {
        std::bitset <1024> bin_rd_data;
        std::stringstream line_stream(line);
        line_stream >> std::hex >> inst_addr;
        line_stream >> std::hex >> ch_idx; 
        line_stream >> std::hex >> data_addr;
        line_stream >> bin_rd_data;
        rd_data = static_cast<uint64_t>(bin_rd_data.to_ullong());
        mem_req_outputs.push_back(rd_data);
    }
    return true;
}

// bool ParseWrReqOutputs(std::vector<uint64_t> &)

/*
    bool ParseOutputs(std::vector<std::vector<int16_t>> &fi_outputs,
                    std::string &io_filename, unsigned int &num_outputs) {
    std::ifstream io_file(io_filename);
    if (!io_file)
        return false;

    int16_t element;
    std::string line;

    std::getline(io_file, line);
    std::stringstream line_stream(line);
    line_stream >> num_outputs;

    while (std::getline(io_file, line)) {
        std::stringstream line_stream(line);
        std::vector<int16_t> tmp;
        while (line_stream.rdbuf()->in_avail() != 0) {
        line_stream >> element;
        tmp.push_back(element);
        }
        fi_outputs.push_back(tmp);
    }
    return true;
    }
*/

bool ParseMemReqs(
    std::vector<data_vector<unsigned int>> &target_channels,
    std::vector<data_vector<uint64_t>> &target_addresses,
    std::vector<std::vector<bool>> &write_ens,
    std::vector<data_vector<size_t>> &write_datas, // however many bits are associated with a single memory write tranaction
    std::vector<data_vector<uint64_t>> &src_ports,
    std::vector<data_vector<uint64_t>> &dst_ports,
    // TODO remove above and just use the mem_inst struct
    // std::vector<data_vector<mem_req_inst>> &mem_req_insts,
    unsigned int &num_mem_req_modules,
    std::string &io_filename) {

    std::ifstream io_file(io_filename);

    if (!io_file)
        return false;

    unsigned int target_channel;
    uint64_t target_address;
    std::string dst_port;
    std::string src_port;
    size_t write_data;
    bool write_enable;

    unsigned int num_inst_addrs; // number of instruction addresses (or memory addresses issued across time)


    // get first line of the mem requests file
    std::string line;
    std::getline(io_file, line);
    std::stringstream header_stream(line);
    header_stream >> num_inst_addrs;

    // Outer vectors are sized to be equal to the number of instruction addresses that exist in the instruction file
    target_channels.resize(num_inst_addrs);
    target_addresses.resize(num_inst_addrs);
    write_ens.resize(num_inst_addrs);
    write_datas.resize(num_inst_addrs);
    src_ports.resize(num_inst_addrs);
    dst_ports.resize(num_inst_addrs);


    unsigned int prev_instruction_addr = 0xfffff;
    unsigned int cur_instruction_addr = 0xfffff;
    bool first_inst = true;


    // We want each data vector to contain the data for a single batch of instructions (i.e. all of the instructions that are issued in the same cycle)
    // The data vectors are sized to be equal to the number of memory request modules as this is size we need for a single batch

    data_vector<unsigned int> target_channel_dvec(num_mem_req_modules);
    data_vector<uint64_t> target_addresses_dvec(num_mem_req_modules);
    std::vector<bool> write_ens_dvec(num_mem_req_modules);
    data_vector<size_t> write_datas_dvec(num_mem_req_modules);
    data_vector<uint64_t> src_ports_dvec(num_mem_req_modules);
    data_vector<uint64_t> dst_ports_dvec(num_mem_req_modules);

    // init to NOP
    for (unsigned int i = 0; i < num_mem_req_modules; i++) {
        target_channel_dvec[i] = 0xfffff;
        target_addresses_dvec[i] = 0xffffff;
        write_ens_dvec[i] = 1;
        write_datas_dvec[i] = 0xfffff;
        src_ports_dvec[i] = 0xffffff;
        dst_ports_dvec[i] = 0xffffff;
    }

    while (std::getline(io_file, line)) {
        std::stringstream line_stream(line);
        /*
            For this parsing we assume that there are at most N instructions per address, where N is the number of memory request modules
            * Instruction addresses are grouped together in batches of N
            * If there exists N memory modules and M instructions s.t M < N then unused indexes of the data vector will be set to NOP opcode
            * NOP = ffff... for all bits in instruction
        */
        
        unsigned int mem_req_module_id;
        prev_instruction_addr = cur_instruction_addr;
        line_stream >> std::hex >> cur_instruction_addr;
        line_stream >> std::dec >> mem_req_module_id;

        assert (num_mem_req_modules > mem_req_module_id); // we cant issue instruction to module that doesnt exist
        line_stream >> src_port >> dst_port >> target_channel;
        line_stream >> std::hex >> target_address;
        line_stream >> write_data >> write_enable;
        
        target_channel_dvec[mem_req_module_id] = target_channel;
        target_addresses_dvec[mem_req_module_id] = target_address;
        write_ens_dvec[mem_req_module_id] = write_enable;
        write_datas_dvec[mem_req_module_id] = write_data;
        src_ports_dvec[mem_req_module_id] = radsim_design.GetPortBaseAddress(src_port);
        dst_ports_dvec[mem_req_module_id] = radsim_design.GetPortBaseAddress(dst_port);

        // instructions have to have batched inst_addresses 
        // so if the cur != prev (or if this is the last line of the file) that means we can push back the values we created onto the vectors of data vectors
        // we stop the first instruction to be pushed because we don't know if its in the same batch as future instructions
        // TODO account for edge case of first instruction of batch N > 1 (this should not be pushed back)
        if ((cur_instruction_addr != prev_instruction_addr || io_file.peek() == EOF)){
            target_channels[cur_instruction_addr] = target_channel_dvec;
            target_addresses[cur_instruction_addr] = target_addresses_dvec;
            write_ens[cur_instruction_addr] = write_ens_dvec;
            write_datas[cur_instruction_addr] = write_datas_dvec;
            src_ports[cur_instruction_addr] = src_ports_dvec;
            dst_ports[cur_instruction_addr] = dst_ports_dvec;
            
            // reset data vectors to NOP
            for (unsigned int i = 0; i < num_mem_req_modules; i++) {
                target_channel_dvec[i] = 0xfffff;
                target_addresses_dvec[i] = 0xffffff;
                write_ens_dvec[i] = 1;
                write_datas_dvec[i] = 0xfffff;
                src_ports_dvec[i] = 0xffffff;
                dst_ports_dvec[i] = 0xffffff;
            }
        }
    }
    return true;
}


hbm_traffic_driver::hbm_traffic_driver( const sc_module_name &name) : sc_module(name) {

    std::vector<hw_module> module_insts;

    // Parse design configuration (number of layers & number of MVM per layer)
    std::string design_root_dir =
        radsim_config.GetStringKnob("radsim_user_design_root_dir");

    std::string mod_insts_fname = design_root_dir + "/compiler/traffic_gen/traffic_gen.cfg";
    ParseTrafficGenModules(module_insts, mod_insts_fname);
    // now we can find the number of black box modules parsed from module isnts
    unsigned int num_mem_req_modules = count_modules(module_insts, "black_box");

    // init sc_vector based ports
    mem_req_valids.init(num_mem_req_modules);
    mem_req_readys.init(num_mem_req_modules);
    wr_ens.init(num_mem_req_modules);


    std::string mem_reqs_fname = design_root_dir + "/compiler/traffic_gen/insts.in";


    // Traffic Gen    
    bool ret = ParseMemReqs(
        _target_channels,
        _target_addresses,
        _wr_ens,
        _wr_datas, // however many bits are associated with a single memory write tranaction
        _src_ports,
        _dst_ports,
        // _mem_req_insts,
        num_mem_req_modules,
        mem_reqs_fname
    );

    if (!ret){
        std::cout << "Couldn't find the traffic generator instruction file!" << std::endl;
    }
    
    // get total number of read and write instructions for each instruction address
    for (unsigned int i = 0; i < _wr_ens.size(); i++){
        unsigned int num_rd_insts, num_wr_insts;
        num_rd_insts = std::count(_wr_ens[i].begin(), _wr_ens[i].end(), false);
        num_wr_insts = std::count(_wr_ens[i].begin(), _wr_ens[i].end(), true);
        _num_rd_insts.push_back(num_rd_insts);
        _num_wr_insts.push_back(num_wr_insts);
    }
    // TODO fix this when dealing with multiple modules and/or multiple instruction addresses
    _total_rd_insts = std::accumulate(_num_rd_insts.begin(), _num_rd_insts.end(), 0,
        [](unsigned int sum, const unsigned int& n){ return sum + n; });
    _total_wr_insts = std::accumulate(_num_wr_insts.begin(), _num_wr_insts.end(), 0,
        [](unsigned int sum, const unsigned int& n){ return sum + n; });
    

    
    std::cout << "Finished parsing traffic gen inputs!" << std::endl;

    std::string mem_req_outputs_basename = design_root_dir + "/compiler/traffic_gen/ddr_ext_mem_ctrl_0_inst_golden";
    std::string rd_req_outputs_fname = mem_req_outputs_basename + "_mem.out";
    std::string wr_req_outputs_fname = mem_req_outputs_basename + "_wr.out";

    unsigned int num_rd_mem_req_outputs, num_wr_mem_req_outputs;
    ParseOutputs(_rd_req_outputs, rd_req_outputs_fname, num_rd_mem_req_outputs);
    ParseOutputs(_wr_req_outputs, wr_req_outputs_fname, num_wr_mem_req_outputs);

    // comment below if you want to screw with instructions 
    // assert( num_rd_mem_req_outputs == _total_rd_insts && num_wr_mem_req_outputs == _total_wr_insts);


    SC_METHOD(assign);
    sensitive << collector_fifo_rdy;
    for (unsigned int i=0; i<mem_req_readys.size(); i++) {
        sensitive << mem_req_readys[i];
    }

    SC_CTHREAD(source, clk.pos());
    SC_CTHREAD(sink, clk.pos()); 
}

hbm_traffic_driver::~hbm_traffic_driver() {}

void hbm_traffic_driver::assign() { 
    collector_fifo_ren.write(collector_fifo_rdy);
}

void hbm_traffic_driver::source() {
    // Reset
    rst.write(true);
    
    // Hbm traffic
    for (unsigned int i=0; i<mem_req_valids.size(); i++){
        mem_req_valids[i].write(false); // TODO make sure this works with the data_vector of valid signals 
    }


    wait();
    rst.write(false);
    wait();

    unsigned int idx = 0;
    _start_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
    // Traffic Gen
    // Assuming all traffic gen inputs are the same size
    while (idx < _target_addresses.size()) { 
        target_channels.write(_target_channels[idx]);
        target_addresses.write(_target_addresses[idx]);
        wr_datas.write(_wr_datas[idx]);
        src_ports.write(_src_ports[idx]);
        dst_ports.write(_dst_ports[idx]);
        // Loop over bools as they are in sc_vectors
        for (unsigned int i=0; i < _wr_ens[idx].size(); i++) {
            // be careful that the valid/ready signals (or other sc_vectors) are initialized to the same number of elements as the data_vectors
            wr_ens[i].write(_wr_ens[idx][i]);
            mem_req_valids[i].write(true);  
        }

        wait();
        // check to make sure all of the ready valids are true if any are false then dont increment idx (keep writing the same transaction)
        bool mem_requesters_ready = std::all_of(mem_req_readys.begin(), mem_req_readys.end(),
            [](const sc_in<bool>& ready_in) {
                return ready_in.read();
            });
        if (mem_requesters_ready){
            idx++;
        }
    }
    // all transactions have been sent, so we set all mem_req valids to false
    for (auto &v : mem_req_valids) {
        v.write(false);
    }

    std::cout << this->name()
                << ": Finished sending all inputs to traffic gen modules!"
                << std::endl;
    wait();
}

void print_progress_bar(unsigned int outputs_count, unsigned int total) {
  unsigned int loading_bar_width = 50;
  std::cout << "[";
  float progress = 1.0 * outputs_count / total;
  unsigned int pos = loading_bar_width * progress;
  for (unsigned int i = 0; i < loading_bar_width; ++i) {
    if (i < pos)
      std::cout << "=";
    else if (i == pos)
      std::cout << ">";
    else
      std::cout << " ";
  }
  if (outputs_count == total) {
    std::cout << "] " << int(progress * 100.0) << " %\n";
  } else {
    std::cout << "] " << int(progress * 100.0) << " %\r";
  }
  std::cout.flush();
}


void hbm_traffic_driver::mem_req_sink_iter(
        RD_WR_FLAG rd_wr_flag,
        unsigned int &outputs_cnt,
        bool &all_outputs_matching
    ){
    // RD = 0, WR = 1
    data_vector<uint64_t> dut_output; // data vector from dut
    std::vector<uint64_t> mem_req_outputs; // vector of outputs generated from compiler
    bool ready;

    if (rd_wr_flag) {
        dut_output = wr_req_data.read();
        ready = wr_req_data_rdy.read();
        mem_req_outputs = _wr_req_outputs;
    } else {
        dut_output = rd_req_data.read();
        ready = rd_req_data_rdy.read();
        mem_req_outputs = _rd_req_outputs;
    }
    if (ready && dut_output.size() > 0) {
        bool matching = true;
        for (unsigned int e = 0; e < dut_output.size(); e++) {
            matching = (dut_output[e] == mem_req_outputs[outputs_cnt]);
        }
        std::string msg_str;
        if (!matching)
            msg_str = " Does Not Match!\n";
        else
            msg_str = " Matches!\n";
        std::cout << "Output " << outputs_cnt << msg_str;
        std::cout << "TRUE: [ ";
        std::cout << mem_req_outputs[outputs_cnt] << " ";
        /*
            for (unsigned int e = 0; e < _mem_req_outputs[outputs_cnt].size(); e++){
                std::cout << _mem_req_outputs[outputs_cnt][e] << " ";
            }
        */
        std::cout << "]\n";
        std::cout << "DUT : [ ";
        for (unsigned int e = 0; e < dut_output.size(); e++) {
            std::cout << dut_output[e] << " ";
        }
        std::cout << "]\n";
        std::cout << "-------------------------------\n";
        outputs_cnt++;
        all_outputs_matching &= matching;

        print_progress_bar(outputs_cnt, _total_rd_insts + _total_wr_insts);
    }
}

void hbm_traffic_driver::sink(){
    std::ofstream mismatching_outputs_file("mismatching.log");
    // unsigned int rd_outputs_cnt = 0;
    // unsigned int wr_outputs_cnt = 0;
    
    // data_vector<uint64_t> rd_dut_output;
    // data_vector<uint64_t> wr_dut_output;

    bool all_outputs_matching = true;
    unsigned int outputs_cnt = 0;
    while (outputs_cnt < _total_rd_insts + _total_wr_insts){
        mem_req_sink_iter(READ, outputs_cnt, all_outputs_matching);
        mem_req_sink_iter(WRITE, outputs_cnt, all_outputs_matching);
        wait();
    }

    std::cout << "Got " << outputs_cnt << " output(s)!\n";
    mismatching_outputs_file.flush();
    mismatching_outputs_file.close();

    if (all_outputs_matching) {
        std::cout << "Simulation PASSED! All outputs matching!" << std::endl;
    } else {
        std::cout << "Simulation FAILED! Some outputs are NOT matching!"
                << std::endl;
    }
    _end_cycle =
        GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
    std::cout << "Simulated " << (_end_cycle - _start_cycle) << " cycle(s)"
                << std::endl;
    std::vector<double> aggregate_bandwidths = NoCTransactionTelemetry::DumpTrafficFlows("/home/hw_sw_flows/third_party/rad-flow/rad-sim/example-designs/hbm_traffic/traffic_flows", _end_cycle - _start_cycle, radsim_design.GetNodeModuleNames());
    std::cout << "Aggregate NoC BW = " << aggregate_bandwidths[0] / 1000000000 << " Gbps" << std::endl;


    for (unsigned int i = 0; i < 10; i++) {
        wait();
    }
    sc_stop();
    NoCTransactionTelemetry::DumpStatsToFile("/home/hw_sw_flows/third_party/rad-flow/rad-sim/example-designs/hbm_traffic/hbm_traffic.csv");

}
/*
void hbm_traffic_driver::sink() {
  std::ofstream mismatching_outputs_file("mismatching.log");

  unsigned int outputs_count = 0;
  data_vector<int16_t> dut_output;
  bool all_outputs_matching = true;
  while (outputs_count < _num_mlp_outputs) {
    dut_output = collector_fifo_rdata.read();
    if (collector_fifo_rdy.read() && dut_output.size() > 0) {
      bool matching = true;
      for (unsigned int e = 0; e < dut_output.size(); e++) {
        matching = (dut_output[e] == _mlp_outputs[outputs_count][e]);
      }
      if (!matching) {
        std::cout << "Output " << outputs_count << " does not match!\n";
        std::cout << "TRUE: [ ";
        for (unsigned int e = 0; e < _mlp_outputs[outputs_count].size(); e++) {
          std::cout << _mlp_outputs[outputs_count][e] << " ";
        }
        std::cout << "]\n";
        std::cout << "DUT : [ ";
        for (unsigned int e = 0; e < dut_output.size(); e++) {
          std::cout << dut_output[e] << " ";
        }
        std::cout << "]\n";
        std::cout << "-------------------------------\n";
      }
      outputs_count++;
      all_outputs_matching &= matching;

      print_progress_bar(outputs_count, _num_mlp_outputs);
    }
    wait();
  }
  std::cout << "Got " << outputs_count << " output(s)!\n";
  mismatching_outputs_file.flush();
  mismatching_outputs_file.close();

  if (all_outputs_matching) {
    std::cout << "Simulation PASSED! All outputs matching!" << std::endl;
  } else {
    std::cout << "Simulation FAILED! Some outputs are NOT matching!"
              << std::endl;
  }
  _end_cycle =
      GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
  std::cout << "Simulated " << (_end_cycle - _start_cycle) << " cycle(s)"
            << std::endl;

  for (unsigned int i = 0; i < 10; i++) {
    wait();
  }
  sc_stop();
}
*/