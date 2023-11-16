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


// TODO update this function to support higher level instantiation of these black boxes from the python compiler
// For now it will just grab the number of consumer_producer insts 
bool ParseTrafficGenModules(
        std::vector<std::string> &module_insts,
        std::string &io_filename) {

    std::ifstream io_file(io_filename);
    if (!io_file)
        return false;

    std::string module_inst;
    std::string line;
    while (getline(io_file, line)) {
        std::stringstream ss(line);
        // looking for module & not endmodule
        if (line.find("module") && !line.find("end")){
            // ignore the first space delim value as its the "module" keyword
            ss.ignore(); // looking for space delim and ignoring it up to max size of string stream
            ss >> module_inst;
        }
        module_insts.push_back(module_inst);
    }
}

bool ParseMemReqs(
    std::vector<data_vector<unsigned int>> &target_channels,
    std::vector<data_vector<uint64_t>> &target_addresses,
    std::vector<std::vector<bool>> &write_ens,
    std::vector<data_vector<size_t>> &write_datas, // however many bits are associated with a single memory write tranaction
    std::vector<data_vector<uint64_t>> &src_ports,
    std::vector<data_vector<uint64_t>> &dst_ports,
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


    unsigned int prev_instruction_addr;
    unsigned int cur_instruction_addr;

    while (std::getline(io_file, line)) {
        std::stringstream line_stream(line);
        unsigned int mem_req_module_id;

        data_vector<unsigned int> target_channel_dvec(num_mem_req_modules);
        data_vector<uint64_t> target_addresses_dvec(num_mem_req_modules);
        std::vector<bool> write_ens_dvec(num_mem_req_modules);
        data_vector<size_t> write_datas_dvec(num_mem_req_modules);
        data_vector<uint64_t> src_ports_dvec(num_mem_req_modules);
        data_vector<uint64_t> dst_ports_dvec(num_mem_req_modules);

        prev_instruction_addr = cur_instruction_addr;
        line_stream >> cur_instruction_addr;
        line_stream >> mem_req_module_id;

        assert (num_mem_req_modules > mem_req_module_id); // we cant issue instruction to module that doesnt exist
        line_stream >> src_port >> dst_port >> target_channel >> target_address >> write_enable >> write_data >> write_enable;
        
        target_channel_dvec[mem_req_module_id] = target_channel;
        target_addresses_dvec[mem_req_module_id] = target_address;
        write_ens_dvec[mem_req_module_id] = write_enable;
        write_datas_dvec[mem_req_module_id] = write_data;
        src_ports_dvec[mem_req_module_id] = radsim_design.GetPortBaseAddress(src_port);
        dst_ports_dvec[mem_req_module_id] = radsim_design.GetPortBaseAddress(dst_port);

        // instructions have to have batched inst_addresses 
        // so if the cur != prev (or if this is the last line of the file) that means we can push back the values we created onto the vectors of data vectors
        if ( cur_instruction_addr != prev_instruction_addr || io_file.peek() == EOF){
            target_channels.push_back(target_channel_dvec);
            target_addresses.push_back(target_addresses_dvec);
            write_ens.push_back(write_ens_dvec);
            write_datas.push_back(write_datas_dvec);
            src_ports.push_back(src_ports_dvec);
            dst_ports.push_back(dst_ports_dvec);
        }
    }
}


hbm_traffic_driver::hbm_traffic_driver( const sc_module_name &name
                                        // unsigned int num_mem_req_insts
                                        ) : sc_module(name) {

    // _num_mem_req_insts = num_mem_req_insts;
    
    // init vectors of ports
    

    std::vector<std::string> module_insts;

    // Parse design configuration (number of layers & number of MVM per layer)
    std::string design_root_dir =
        radsim_config.GetStringKnob("radsim_user_design_root_dir");

    std::string mod_insts_fname = design_root_dir + "/compiler/traffic_gen/traffic_gen.cfg";
    ParseTrafficGenModules(module_insts, mod_insts_fname);
    // now we can find the number of black box modules parsed from module isnts
    unsigned int num_mem_req_modules = count(module_insts.begin(), module_insts.end(), "black_box");

    // init sc_vector based ports
    mem_req_valids.init(num_mem_req_modules);
    mem_req_readys.init(num_mem_req_modules);


    std::string mem_reqs_fname = design_root_dir + "/compiler/traffic_gen/isnts.in";
    
    // Traffic Gen    
    ParseMemReqs(
        _target_channels,
        _target_addresses,
        _wr_ens,
        _wr_datas, // however many bits are associated with a single memory write tranaction
        _src_ports,
        _dst_ports,
        num_mem_req_modules,
        mem_reqs_fname
    );
    
    
    std::cout << "Finished parsing traffic gen inputs!" << std::endl;

    std::string inputs_filename =
        design_root_dir + "/compiler/embedding_indecies.in";
    ParseInputs(_lookup_indecies, _target_channels, _base_addresses,
                inputs_filename);
    std::cout << "Finished parsing inputs!" << std::endl;

    std::string feature_interaction_outputs_filename =
        design_root_dir + "/compiler/feature_interaction.out";
    ParseOutputs(_feature_interaction_outputs,
                feature_interaction_outputs_filename,
                _num_feature_interaction_outputs);

    std::string mlp_outputs_filename = design_root_dir + "/compiler/mlp.out";
    ParseOutputs(_mlp_outputs, mlp_outputs_filename, _num_mlp_outputs);
    SC_METHOD(assign);
    sensitive << collector_fifo_rdy;
    for (int i=0; i<mem_req_readys.size(); i++) {
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
    lookup_indecies_valid.write(false);
    // Hbm traffic
    for (int i=0; i<mem_req_valids.size(); i++){
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
        for (int i=0; i < _wr_ens[idx].size(); i++) {
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


    while (idx < _lookup_indecies.size()) {
        lookup_indecies_data.write(_lookup_indecies[idx]);
        lookup_indecies_target_channels.write(_target_channels[idx]);
        lookup_indecies_base_addresses.write(_base_addresses[idx]);
        lookup_indecies_valid.write(true);

        wait();

        if (lookup_indecies_valid.read() && lookup_indecies_ready.read()) {
        idx++;
        }
    }
    
    lookup_indecies_valid.write(false);
    
    std::cout << this->name()
                << ": Finished sending all inputs to traffic gen modules!"
                << std::endl;

    /*
        std::cout << this->name()
                    << ": Finished sending all inputs to embedding lookup module!"
                    << std::endl;
    */
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