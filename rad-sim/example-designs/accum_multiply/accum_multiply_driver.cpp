#include <aximm_hello_world_driver.hpp>

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
    } else {
      c.type = false;
    }
    sim_trace.push_back(c);
  }
  return true;
}

aximm_hello_world_driver::aximm_hello_world_driver(const sc_module_name &name)
    : sc_module(name) {

  // Parse design configuration (number of layers & number of MVM per layer)
  std::string design_root_dir =
      radsim_config.GetStringKnob("radsim_user_design_root_dir");

  std::string sim_trace_filename = design_root_dir + "/sim_trace";
  ParseTrace(hello_world_sim_trace, sim_trace_filename);
  std::cout << "Finished parsing simulation trace ..." << std::endl;
  for (unsigned int i = 0; i < hello_world_sim_trace.size(); i++)
    std::cout << hello_world_sim_trace[i].cycle << " "
              << hello_world_sim_trace[i].type << " "
              << hello_world_sim_trace[i].addr << " "
              << hello_world_sim_trace[i].data << std::endl;
  std::cout << "----------------------------------------" << std::endl;

  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

aximm_hello_world_driver::~aximm_hello_world_driver() {}

void aximm_hello_world_driver::source() {
  // Reset
  rst.write(true);
  req_valid.write(false);
  wait();
  rst.write(false);
  wait();

  unsigned int idx = 0;
  while (idx < hello_world_sim_trace.size()) {
    req_addr.write(hello_world_sim_trace[idx].addr);
    req_type.write(hello_world_sim_trace[idx].type);
    req_wdata.write(hello_world_sim_trace[idx].data);
    req_valid.write(true);

    wait();

    if (req_valid.read() && req_ready.read()) {
      idx++;
    }
  }
  req_valid.write(false);
  std::cout << "Finished sending all traces to requester module!" << std::endl;
  wait();
}

void aximm_hello_world_driver::sink() {
  while (received_responses.read() < hello_world_sim_trace.size()) {
    wait();
  }
  std::cout << "Received all " << received_responses.read() << " responses!"
            << std::endl;
  sc_stop();
}