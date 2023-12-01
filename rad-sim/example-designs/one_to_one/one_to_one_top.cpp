#include <one_to_one_top.hpp>

one_to_one_top::one_to_one_top(const sc_module_name &name) : sc_module(name) {

  unsigned int line_bitwidth = 512;
  unsigned int element_bitwidth = 16;
  std::vector<unsigned int> mem_channels = {1, 1, 8, 8};
  unsigned int embedding_lookup_fifos_depth = 16;
  unsigned int feature_interaction_fifos_depth = 64;
  unsigned int num_mem_controllers =
      radsim_config.GetIntKnob("dram_num_controllers");
  assert(num_mem_controllers == mem_channels.size());
  unsigned int total_mem_channels = 0;
  for (auto &num_channels : mem_channels) {
    total_mem_channels += num_channels;
  }

  std::string module_name_str;
  char module_name[25];


  // Instantiate Embedding Lookup Module
  module_name_str = "embedding_lookup_inst";
  std::strcpy(module_name, module_name_str.c_str());
  embedding_lookup_inst = new embedding_lookup(
      module_name, line_bitwidth, mem_channels, embedding_lookup_fifos_depth);
  embedding_lookup_inst->rst(rst);
  embedding_lookup_inst->lookup_indecies_data(lookup_indecies_data);
  embedding_lookup_inst->lookup_indecies_target_channels(
      lookup_indecies_target_channels);
  embedding_lookup_inst->lookup_indecies_base_addresses(
      lookup_indecies_base_addresses);
  embedding_lookup_inst->lookup_indecies_valid(lookup_indecies_valid);
  embedding_lookup_inst->lookup_indecies_ready(lookup_indecies_ready);

  // Instantiate Feature Interaction Module
  module_name_str = "feature_interaction_inst";
  std::strcpy(module_name, module_name_str.c_str());
  std::string feature_interaction_inst_file =
      radsim_config.GetStringKnob("radsim_user_design_root_dir") +
      "/instructions/feature_interaction.inst";
  feature_interaction_inst = new custom_feature_interaction(
      module_name, line_bitwidth, element_bitwidth, total_mem_channels,
      feature_interaction_fifos_depth, num_mvms[0],
      feature_interaction_inst_file);
  feature_interaction_inst->rst(rst);
  feature_interaction_inst->received_responses(received_responses);


  axis_sig.resize(axis_signal_count);
  unsigned int idx = 0;
  for (unsigned int l = 0; l < num_layers; l++) {
    for (unsigned int m = 0; m < num_mvms[l]; m++) {
      if (m == num_mvms[l] - 1 && l == num_layers - 1) {
        axis_sig[idx].Connect(mvms[l][m]->tx_reduce_interface,
                              mvms[0][0]->rx_reduce_interface);
      } else if (m == num_mvms[l] - 1) {
        axis_sig[idx].Connect(mvms[l][m]->tx_reduce_interface,
                              mvms[l + 1][0]->rx_reduce_interface);
      } else {
        axis_sig[idx].Connect(mvms[l][m]->tx_reduce_interface,
                              mvms[l][m + 1]->rx_reduce_interface);
      }
      idx++;
    }
  }

  // Instantiate Output Collector
  module_name_str = "output_collector";
  std::strcpy(module_name, module_name_str.c_str());
  output_collector = new collector(module_name);
  output_collector->rst(rst);
  output_collector->data_fifo_rdy(collector_fifo_rdy);
  output_collector->data_fifo_ren(collector_fifo_ren);
  output_collector->data_fifo_rdata(collector_fifo_rdata);

  ext_mem.resize(num_mem_controllers);
  mem_clks.resize(num_mem_controllers);
  unsigned int ch_id = 0;
  std::string mem_content_init_prefix =
      radsim_config.GetStringKnob("radsim_user_design_root_dir") +
      "/compiler/embedding_tables/channel_";
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

  radsim_design.BuildDesignContext("one_to_one.place", "one_to_one.clks");
  radsim_design.CreateSystemNoCs(rst);
  radsim_design.ConnectModulesToNoC();
}

one_to_one_top::~one_to_one_top() {
  delete embedding_lookup_inst;
  delete feature_interaction_inst;
  for (auto &ctrlr : ext_mem)
    delete ctrlr;
  delete output_collector;
  for (unsigned int l = 0; l < mvms.size(); l++) {
    for (auto &mvm : mvms[l]) {
      delete mvm;
    }
  }
}