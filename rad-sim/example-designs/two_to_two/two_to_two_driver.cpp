#include <two_to_two_driver.hpp>

// int flag = 0;
// int counter = 0;

bool ParseTrace(std::vector<Command> &sim_trace, std::string &io_filename) {
  std::ifstream io_file(io_filename);
  if (!io_file)
    return false;

  std::string line, cmd;
  while (std::getline(io_file, line)) {
    std::stringstream line_stream(line);
    Command c;
    line_stream >> c.cycle;
    line_stream >> cmd;
    line_stream >> c.addr;
    if (cmd == "WRITE") {
      line_stream >> c.data;
      c.type = true;
      // flag = 1;
    } else {
      c.type = false;
    }
    sim_trace.push_back(c);
  }
  return true;
}

two_to_two_driver::two_to_two_driver(const sc_module_name &name)
    : sc_module(name) {

  // Parse design configuration (number of layers & number of MVM per layer)
  std::string design_root_dir =
      radsim_config.GetStringKnob("radsim_user_design_root_dir");

  std::string sim_trace_filename_0 = design_root_dir + "/sim_trace_0";
  ParseTrace(two_to_two_sim_trace, sim_trace_filename_0);
  std::string sim_trace_filename_1 = design_root_dir + "/sim_trace_1";
  ParseTrace(two_to_two_sim_trace, sim_trace_filename_1);
  std::cout << "Finished parsing simulation trace ..." << std::endl;
  // std::cout << std::to_string(two_to_two_sim_trace.size()) << std::endl;
  for (unsigned int i = 0; i < two_to_two_sim_trace.size(); i++)
    std::cout << two_to_two_sim_trace[i].cycle << " "
              << two_to_two_sim_trace[i].type << " "
              << two_to_two_sim_trace[i].addr << " "
              << two_to_two_sim_trace[i].data << std::endl;

  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

two_to_two_driver::~two_to_two_driver() {}

void two_to_two_driver::source() {
  // Reset
  rst.write(true);
  req_valid.write(false);
  req_valid_2.write(false);
  wait();
  rst.write(false);
  start_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
  start_time = std::chrono::steady_clock::now();
  wait();
  std::cout << "Finished parsing simulation trace ..." << std::endl;

  unsigned int idx = 0;
  while (idx < two_to_two_sim_trace.size()/2) {
    req_addr.write(two_to_two_sim_trace[idx].addr);
    req_type.write(two_to_two_sim_trace[idx].type);
    req_wdata.write(two_to_two_sim_trace[idx].data);
    req_valid.write(true);
    req_addr_2.write(two_to_two_sim_trace[idx+4].addr);
    req_type_2.write(two_to_two_sim_trace[idx+4].type);
    req_wdata_2.write(two_to_two_sim_trace[idx+4].data);
    req_valid_2.write(true);

    wait();

    if (req_valid.read() && req_ready.read() && req_valid_2.read() && req_ready_2.read()) {
      idx++;
    }
  }
  // while (idx < two_to_two_sim_trace.size()/2) {
  // req_addr_2.write(two_to_two_sim_trace[idx].addr);
  // req_type_2.write(two_to_two_sim_trace[idx].type);
  // req_wdata_2.write(two_to_two_sim_trace[idx].data);
  // req_valid_2.write(true);

  // wait();

  // if (req_valid_2.read() && req_ready_2.read()) {
  //   idx++;
  // }
  // }
  req_valid.write(false);
  req_valid_2.write(false);
  std::cout << "Finished sending all traces to traffic_generator module!" << std::endl;
  wait();
}

void two_to_two_driver::sink() {
  while ((received_responses.read() + received_responses_2.read()) < two_to_two_sim_trace.size()) {
    std::cout << "Received: " << received_responses.read() <<  " " << received_responses_2.read() << " responses!"
            << std::endl;
    wait();
  }
  std::cout << "Received all " << (received_responses.read() + received_responses_2.read()) << " responses!"
            << std::endl;

  end_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
  end_time = std::chrono::steady_clock::now();
  std::cout << "Simulation Cycles = " << end_cycle - start_cycle << std::endl;
  std::cout << "Simulation Time = " << std::chrono::duration_cast<std::chrono::microseconds> (end_time - start_time).count() << " us" << std::endl;
  NoCTransactionTelemetry::DumpStatsToFile("stats.csv");

  sc_stop();
}