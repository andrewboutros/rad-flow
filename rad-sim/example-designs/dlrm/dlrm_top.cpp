#include <dlrm_top.hpp>

dlrm_top::dlrm_top(const sc_module_name &name) : sc_module(name) {

  std::string module_name_str;
  char module_name[25];

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

  module_name_str = "feature_interaction_inst";
  std::strcpy(module_name, module_name_str.c_str());
  std::string feature_interaction_inst_file =
      radsim_config.GetStringKnob("radsim_user_design_root_dir") +
      "/compiler/instructions/feature_interaction.inst";
  feature_interaction_inst = new feature_interaction(
      module_name, 512, 16, 8, 64, feature_interaction_inst_file);
  feature_interaction_inst->rst(rst);
  feature_interaction_inst->feature_interaction_odata(
      feature_interaction_odata);
  feature_interaction_inst->feature_interaction_valid(
      feature_interaction_valid);
  feature_interaction_inst->feature_interaction_ready(
      feature_interaction_ready);
  feature_interaction_inst->received_responses(received_responses);

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