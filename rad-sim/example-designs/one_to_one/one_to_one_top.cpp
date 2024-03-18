#include <one_to_one_top.hpp>

one_to_one_top::one_to_one_top(const sc_module_name &name)
    : sc_module(name) {

  std::string module_name_str;
  char module_name[25];

  module_name_str = "traffic_generator_inst";
  std::strcpy(module_name, module_name_str.c_str());

  traffic_generator_inst = new traffic_generator(module_name, 16);
  traffic_generator_inst->rst(rst);
  traffic_generator_inst->req_addr(req_addr);
  traffic_generator_inst->req_type(req_type);
  traffic_generator_inst->req_wdata(req_wdata);
  traffic_generator_inst->req_valid(req_valid);
  traffic_generator_inst->req_ready(req_ready);
  traffic_generator_inst->received_responses(received_responses);

  std::string mem_content_init_prefix =
      radsim_config.GetStringKnob("radsim_user_design_root_dir") +
      "/channel_";
  double mem_clk_period =
      radsim_config.GetDoubleVectorKnob("dram_clk_periods", 0);
  module_name_str = "ext_mem_0_clk";
  std::strcpy(module_name, module_name_str.c_str());
  mem_clks = new sc_clock(module_name, mem_clk_period, SC_NS);
  module_name_str = "ext_mem_0";
  std::strcpy(module_name, module_name_str.c_str());
  std::string mem_content_init = mem_content_init_prefix + to_string(0);
  ext_mem = new mem_controller(module_name, 0, mem_content_init);
  ext_mem->mem_clk(*mem_clks);
  ext_mem->rst(rst);
  // ext_mem->mem_channels[0].arlen.read();
  // ext_mem->mem_channels[0].awlen.read();
  // std::cout << "come on.... " << boost::lexical_cast<std::string>(traffic_generator_inst->aximm_req_interface.awlen.read()) << std::endl;


  radsim_design.BuildDesignContext("one_to_one.place",
                                   "one_to_one.clks");
  // std::cout << "come on.... " << boost::lexical_cast<std::string>(traffic_generator_inst->aximm_req_interface.awlen.read()) << std::endl;
  radsim_design.CreateSystemNoCs(rst);
  radsim_design.ConnectModulesToNoC();
}

one_to_one_top::~one_to_one_top() {
  delete traffic_generator_inst;
  delete ext_mem;
  delete mem_clks;
}