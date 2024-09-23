#include <mlp_top.hpp>

mlp_top::mlp_top(const sc_module_name &name, RADSimDesignContext* radsim_design) : RADSimDesignTop(radsim_design) {
  this->radsim_design = radsim_design;
  std::string design_root_dir =
      radsim_config.GetStringKnobPerRad("radsim_user_design_root_dir", radsim_design->rad_id);
  std::string design_config_filename =
      design_root_dir + "/compiler/layer_mvm_config";

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

  init_vector<sc_out<bool>>::init_sc_vector(dispatcher_fifo_rdy, num_mvms[0]);
  init_vector<sc_in<bool>>::init_sc_vector(dispatcher_fifo_wen, num_mvms[0]);
  init_vector<sc_in<data_vector<sc_int<32>>>>::init_sc_vector(
      dispatcher_fifo_wdata, num_mvms[0]);

  matrix_vector_engines.resize(num_layers);
  input_dispatchers.resize(num_mvms[0]);
  char module_name[25];
  std::string module_name_str, inst_filename;
  for (unsigned int layer_id = 0; layer_id < num_layers; layer_id++) {
    matrix_vector_engines[layer_id].resize(num_mvms[layer_id]);
    for (unsigned int mvm_id = 0; mvm_id < num_mvms[layer_id]; mvm_id++) {
      module_name_str =
          "layer" + std::to_string(layer_id) + "_mvm" + std::to_string(mvm_id);
      inst_filename =
          design_root_dir + "/compiler/inst_mifs/" + module_name_str + ".mif";
      std::strcpy(module_name, module_name_str.c_str());
      matrix_vector_engines[layer_id][mvm_id] =
          new mvm(module_name, mvm_id, layer_id, inst_filename, radsim_design);
      matrix_vector_engines[layer_id][mvm_id]->rst(rst);

      if (layer_id == 0) {
        module_name_str = "input_dispatcher" + std::to_string(mvm_id);
        std::strcpy(module_name, module_name_str.c_str());
        input_dispatchers[mvm_id] = new dispatcher(module_name, mvm_id, radsim_design);
        input_dispatchers[mvm_id]->rst(rst);
        input_dispatchers[mvm_id]->data_fifo_rdy(dispatcher_fifo_rdy[mvm_id]);
        input_dispatchers[mvm_id]->data_fifo_wen(dispatcher_fifo_wen[mvm_id]);
        input_dispatchers[mvm_id]->data_fifo_wdata(
            dispatcher_fifo_wdata[mvm_id]);
      }
    }
  }

  module_name_str = "output_collector";
  std::strcpy(module_name, module_name_str.c_str());
  output_collector = new collector(module_name, radsim_design);
  output_collector->rst(rst);
  output_collector->data_fifo_rdy(collector_fifo_rdy);
  output_collector->data_fifo_ren(collector_fifo_ren);
  output_collector->data_fifo_rdata(collector_fifo_rdata);

  #ifndef SINGLE_RAD
  this->portal_inst->rst(rst);
  #endif
  radsim_design->BuildDesignContext("mlp.place", "mlp.clks");
  radsim_design->CreateSystemNoCs(rst);
  radsim_design->ConnectModulesToNoC();
}

mlp_top::~mlp_top() {
  for (unsigned int layer_id = 0; layer_id < matrix_vector_engines.size();
       layer_id++) {
    for (unsigned int mvm_id = 0;
         mvm_id < matrix_vector_engines[layer_id].size(); mvm_id++) {
      delete matrix_vector_engines[layer_id][mvm_id];
      if (layer_id == 0)
        delete input_dispatchers[mvm_id];
    }
  }
  delete output_collector;
}