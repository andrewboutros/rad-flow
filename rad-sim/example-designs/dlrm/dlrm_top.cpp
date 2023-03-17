#include <dlrm_top.hpp>

dlrm_top::dlrm_top(const sc_module_name &name) : sc_module(name) {

  std::string module_name_str;
  char module_name[25];

  // Parse MVM configuration
  std::string design_root_dir =
      radsim_config.GetStringKnob("radsim_user_design_root_dir");
  std::string design_config_filename =
      design_root_dir + "/compiler/mvms.config";

  std::ifstream design_config_file(design_config_filename);
  if (!design_config_file) {
    std::cerr << "Cannot read MLP design configuration file!" << std::endl;
    exit(1);
  }
  std::string line;
  std::getline(design_config_file, line);
  std::stringstream line_stream(line);
  unsigned int num_layers, tmp;
  std::vector<unsigned int> num_mvms;
  line_stream >> num_layers;
  num_mvms.resize(num_layers);
  for (unsigned int layer_id = 0; layer_id < num_layers; layer_id++) {
    line_stream >> tmp;
    num_mvms[layer_id] = tmp;
  }

  // Instantiate Embedding Lookup Module
  module_name_str = "embedding_lookup_inst";
  std::strcpy(module_name, module_name_str.c_str());
  embedding_lookup_inst = new embedding_lookup(module_name, 512, 8, 1, 16);
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
      "/compiler/instructions/feature_interaction.inst";
  feature_interaction_inst = new feature_interaction(
      module_name, 512, 16, 8, 64, num_mvms[0], feature_interaction_inst_file);
  feature_interaction_inst->rst(rst);
  feature_interaction_inst->received_responses(received_responses);

  // Instantiate MVM Engines

  unsigned int axis_signal_count = 0;
  mvms.resize(num_layers);
  for (unsigned int l = 0; l < num_layers; l++) {
    mvms[l].resize(num_mvms[l]);
    for (unsigned int m = 0; m < num_mvms[l]; m++) {
      module_name_str = "layer" + to_string(l) + "_mvm" + to_string(m);
      std::strcpy(module_name, module_name_str.c_str());
      std::string inst_filename = design_root_dir + "/compiler/instructions/" +
                                  module_name_str + ".inst";
      mvms[l][m] = new mvm(module_name, m, l, inst_filename);
      mvms[l][m]->rst(rst);
      axis_signal_count++;
    }
  }

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

  module_name_str = "ext_mem_0";
  std::strcpy(module_name, module_name_str.c_str());
  std::string mem_content_init =
      radsim_config.GetStringKnob("radsim_user_design_root_dir") +
      "/compiler/embedding_tables/channel_0";
  ext_mem = new mem_controller(module_name, 0, mem_content_init);
  ext_mem->mem_clk(mem_clk);
  ext_mem->rst(rst);

  radsim_design.BuildDesignContext("dlrm.place", "dlrm.clks");
  radsim_design.CreateSystemNoCs(rst);
  radsim_design.ConnectModulesToNoC();
}

dlrm_top::~dlrm_top() {
  delete embedding_lookup_inst;
  delete feature_interaction_inst;
  delete ext_mem;
}