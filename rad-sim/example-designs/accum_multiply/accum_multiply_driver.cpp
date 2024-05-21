#include <accum_multiply_driver.hpp>

bool ParseTrace(std::vector<Command> &sim_trace, std::string &io_filename) {
  std::ifstream io_file(io_filename);
  if (!io_file)
    return false;

  std::string line, cmd;
  while (std::getline(io_file, line)) {
    std::stringstream line_stream(line);
    Command c;
    line_stream >> c.cycle;
    line_stream >> c.value;
    sim_trace.push_back(c);
  }
  return true;
}

accum_multiply_driver::accum_multiply_driver(const sc_module_name &name)
    : sc_module(name) {

  // Parse design configuration (number of layers & number of MVM per layer)
  std::string design_root_dir =
      radsim_config.GetStringKnob("radsim_user_design_root_dir");

  std::string sim_trace_filename = design_root_dir + "/sim_trace";
  ParseTrace(accum_multiply_sim_trace, sim_trace_filename);
  std::cout << "Finished parsing simulation trace ..." << std::endl;
  for (unsigned int i = 0; i < accum_multiply_sim_trace.size(); i++)
    std::cout << accum_multiply_sim_trace[i].cycle << " "
              << accum_multiply_sim_trace[i].value << std::endl;
  std::cout << "----------------------------------------" << std::endl;

  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

accum_multiply_driver::~accum_multiply_driver() {}

void accum_multiply_driver::source() {
  // Reset
  rst.write(true);
  source_wdata.write(0);
  source_valid.write(false);
  wait();
  rst.write(false);
  wait();

  unsigned int idx = 0;
  while (idx < accum_multiply_sim_trace.size()) {
    source_wdata.write(accum_multiply_sim_trace[idx].value);
    std::cout << "driver write " << accum_multiply_sim_trace[idx].value << endl;
    source_valid.write(true);

    wait();

    if (source_valid.read() && source_ready.read()) {
      idx++;
    }
  }
  source_valid.write(false);
  std::cout << "Finished sending all traces to requester module!" << std::endl;
  // exit(1);
  wait();
}

void accum_multiply_driver::sink() {
  // Reset
  sink_ready.write(false);
  wait();

  unsigned int idx = 0;
  while (idx < accum_multiply_sim_trace.size()) {
    sink_ready.write(true);
    wait();
    if (sink_valid.read() && sink_ready.read()) {
      std::cout << "rdata " << accum_multiply_sim_trace[idx].cycle << " "
              << sink_rdata.read() << std::endl;
      idx++;
    }
  }
  sink_ready.write(false);
  std::cout << "Received responses!"
            << std::endl;
  sc_stop();
}