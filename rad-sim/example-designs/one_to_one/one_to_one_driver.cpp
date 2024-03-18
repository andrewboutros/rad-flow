#include <one_to_one_driver.hpp>

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

one_to_one_driver::one_to_one_driver(const sc_module_name &name)
    : sc_module(name) {

  // Parse design configuration (number of layers & number of MVM per layer)
  std::string design_root_dir =
      radsim_config.GetStringKnob("radsim_user_design_root_dir");

  std::string sim_trace_filename = design_root_dir + "/sim_trace";
  ParseTrace(one_to_one_sim_trace, sim_trace_filename);
  std::cout << "Finished parsing simulation trace ..." << std::endl;
  // std::cout << std::to_string(one_to_one_sim_trace.size()) << std::endl;
  for (unsigned int i = 0; i < one_to_one_sim_trace.size(); i++)
    std::cout << one_to_one_sim_trace[i].cycle << " "
              << one_to_one_sim_trace[i].type << " "
              << one_to_one_sim_trace[i].addr << " "
              << one_to_one_sim_trace[i].data << std::endl;

  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

one_to_one_driver::~one_to_one_driver() {}

void one_to_one_driver::source() {
  // Reset
  rst.write(true);
  req_valid.write(false);
  wait();
  rst.write(false);
  start_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
  start_time = std::chrono::steady_clock::now();
  wait();

  unsigned int idx = 0;
  while (idx < one_to_one_sim_trace.size()) {
    req_addr.write(one_to_one_sim_trace[idx].addr);
    req_type.write(one_to_one_sim_trace[idx].type);
    req_wdata.write(one_to_one_sim_trace[idx].data);
    req_valid.write(true);

    wait();

    if (req_valid.read() && req_ready.read()) {
      idx++;
    }
  }
  req_valid.write(false);
  std::cout << "Finished sending all traces to traffic_generator module!" << std::endl;
  wait();
}

void one_to_one_driver::sink() {
  while (received_responses.read() < one_to_one_sim_trace.size()) {
    wait();
  }
  std::cout << "Received all " << received_responses.read() << " responses!"
            << std::endl;
  // if (flag==1) {
    
  //   ofstream myfile;
  //   myfile.open (radsim_config.GetStringKnob("radsim_user_design_root_dir") +"/channel_0.dat");
  //   myfile << std::to_string(one_to_one_sim_trace[counter].addr) << " " << one_to_one_sim_trace[counter].data << std::endl;
  //   myfile.close();
  //   counter++;
  // }
  end_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
  end_time = std::chrono::steady_clock::now();
  std::cout << "Simulation Cycles = " << end_cycle - start_cycle << std::endl;
  std::cout << "Simulation Time = " << std::chrono::duration_cast<std::chrono::microseconds> (end_time - start_time).count() << " us" << std::endl;
  NoCTransactionTelemetry::DumpStatsToFile("stats.csv");

  sc_stop();
}