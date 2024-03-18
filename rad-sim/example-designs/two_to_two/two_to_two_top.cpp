#include <two_to_two_top.hpp>

two_to_two_top::two_to_two_top(const sc_module_name &name)
    : sc_module(name) {

  std::vector<unsigned int> mem_channels = {1, 1};

  unsigned int num_mem_controllers =
      radsim_config.GetIntKnob("dram_num_controllers");
  assert(num_mem_controllers == mem_channels.size());
  // unsigned int total_mem_channels = 0;

  std::string module_name_str;
  char module_name[25];

  module_name_str = "traffic_generator_inst_0";
  std::strcpy(module_name, module_name_str.c_str());

  traffic_generator_inst_0 = new traffic_generator(module_name, 16, "0");
  traffic_generator_inst_0->rst(rst);
  traffic_generator_inst_0->req_addr(req_addr);
  traffic_generator_inst_0->req_type(req_type);
  traffic_generator_inst_0->req_wdata(req_wdata);
  traffic_generator_inst_0->req_valid(req_valid);
  traffic_generator_inst_0->req_ready(req_ready);
  traffic_generator_inst_0->received_responses(received_responses);

  module_name_str = "traffic_generator_inst_1";
  std::strcpy(module_name, module_name_str.c_str());

  traffic_generator_inst_1 = new traffic_generator(module_name, 16, "1");
  traffic_generator_inst_1->rst(rst2);
  traffic_generator_inst_1->req_addr(req_addr2);
  traffic_generator_inst_1->req_type(req_type2);
  traffic_generator_inst_1->req_wdata(req_wdata2);
  traffic_generator_inst_1->req_valid(req_valid2);
  traffic_generator_inst_1->req_ready(req_ready2);
  traffic_generator_inst_1->received_responses(received_responses2);

  ext_mem.resize(num_mem_controllers);
  mem_clks.resize(num_mem_controllers);
  unsigned int ch_id = 0;
  std::string mem_content_init_prefix =
      radsim_config.GetStringKnob("radsim_user_design_root_dir") + "/channel_";
  for (unsigned int ctrl_id = 0; ctrl_id < num_mem_controllers; ctrl_id++) {
    double mem_clk_period =
        radsim_config.GetDoubleVectorKnob("dram_clk_periods", ctrl_id);
    module_name_str = "ext_mem_" + to_string(ctrl_id) + "_clk";
    std::strcpy(module_name, module_name_str.c_str());
    mem_clks[ctrl_id] = new sc_clock(module_name, mem_clk_period, SC_NS);
    module_name_str = "ext_mem_" + to_string(ctrl_id);
    std::strcpy(module_name, module_name_str.c_str());
    std::string mem_content_init = mem_content_init_prefix + to_string(ch_id);
    ext_mem[ctrl_id] =
        new mem_controller(module_name, ctrl_id, mem_content_init);
    ext_mem[ctrl_id]->mem_clk(*mem_clks[ctrl_id]);
    ext_mem[ctrl_id]->rst(rst);
    ch_id += mem_channels[ctrl_id];
  }

  radsim_design.BuildDesignContext("two_to_two.place",
                                   "two_to_two.clks");

  radsim_design.CreateSystemNoCs(rst);
  radsim_design.ConnectModulesToNoC();
}

two_to_two_top::~two_to_two_top() {
  delete traffic_generator_inst_0;
  delete traffic_generator_inst_1;
  for (auto &ctrlr : ext_mem)
    delete ctrlr;
  for (auto &mem_clk : mem_clks)
    delete mem_clk;
}