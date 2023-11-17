#include <mlp_top.hpp>

mlp_top::mlp_top(const sc_module_name &name) : sc_module(name) {

  std::string design_root_dir =
      radsim_config.GetStringKnob("radsim_user_design_root_dir");
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
  unsigned int num_layers;
  std::string num_mvms_layer, num_mvms_rtl_layer;
  std::string layer_mvms;
  line_stream >> num_layers;
  num_mvms.resize(num_layers);
  num_mvms_rtl.resize(num_layers);
  num_mvms_total.resize(num_layers);
  for (unsigned int layer_id = 0; layer_id < num_layers; layer_id++) {
    line_stream >> layer_mvms;
    std::stringstream layer_mvms_stream(layer_mvms);
    std::getline(layer_mvms_stream, num_mvms_layer, ',');
    std::getline(layer_mvms_stream, num_mvms_rtl_layer, ',');
    num_mvms[layer_id] = std::stoi(num_mvms_layer);
    num_mvms_rtl[layer_id] = std::stoi(num_mvms_rtl_layer);
    num_mvms_total[layer_id] = num_mvms[layer_id] + num_mvms_rtl[layer_id];
  }

  init_vector<sc_out<bool>>::init_sc_vector(dispatcher_fifo_rdy, num_mvms_total[0]);
  init_vector<sc_in<bool>>::init_sc_vector(dispatcher_fifo_wen, num_mvms_total[0]);
  init_vector<sc_in<data_vector<sc_int<IPRECISION>>>>::init_sc_vector(
      dispatcher_fifo_wdata, num_mvms_total[0]);

  matrix_vector_engines.resize(num_layers);
  rtl_matrix_vector_engines.resize(num_layers);
  input_dispatchers.resize(num_mvms_total[0]);
  char module_name[25];
  std::string module_name_str;
  for (unsigned int layer_id = 0; layer_id < num_layers; layer_id++) {
    matrix_vector_engines[layer_id].resize(num_mvms[layer_id]);
    rtl_matrix_vector_engines[layer_id].resize(num_mvms_rtl[layer_id]);
    for (unsigned int mvm_id = 0; mvm_id < num_mvms[layer_id]; mvm_id++) {
      module_name_str =
          "layer" + std::to_string(layer_id) + "_mvm" + std::to_string(mvm_id);
      std::strcpy(module_name, module_name_str.c_str());
      matrix_vector_engines[layer_id][mvm_id] =
          new mvm(module_name, mvm_id, layer_id);
      matrix_vector_engines[layer_id][mvm_id]->rst(rst);
    }
    for (unsigned int mvm_id = 0; mvm_id < num_mvms_rtl[layer_id]; mvm_id++) {
      module_name_str =
          "layer" + std::to_string(layer_id) + "_mvm" + std::to_string(mvm_id + num_mvms[layer_id]);
      std::strcpy(module_name, module_name_str.c_str());
      rtl_matrix_vector_engines[layer_id][mvm_id] =
          new rtl_mvm(module_name);
      rtl_matrix_vector_engines[layer_id][mvm_id]->rst(rst);
    }
  }

  for (unsigned int mvm_id = 0; mvm_id < num_mvms_total[0]; mvm_id++) {
      module_name_str = "input_dispatcher" + std::to_string(mvm_id);
      std::strcpy(module_name, module_name_str.c_str());
      input_dispatchers[mvm_id] = new dispatcher(module_name, mvm_id);
      input_dispatchers[mvm_id]->rst(rst);
      input_dispatchers[mvm_id]->data_fifo_rdy(dispatcher_fifo_rdy[mvm_id]);
      input_dispatchers[mvm_id]->data_fifo_wen(dispatcher_fifo_wen[mvm_id]);
      input_dispatchers[mvm_id]->data_fifo_wdata(
          dispatcher_fifo_wdata[mvm_id]);
  }

  module_name_str = "output_collector";
  std::strcpy(module_name, module_name_str.c_str());
  output_collector = new collector(module_name);
  output_collector->rst(rst);
  output_collector->data_fifo_rdy(collector_fifo_rdy);
  output_collector->data_fifo_ren(collector_fifo_ren);
  output_collector->data_fifo_rdata(collector_fifo_rdata);

  module_name_str = "weight_loader";
  std::strcpy(module_name, module_name_str.c_str());
  wloader = new weight_loader(module_name);
  wloader->rst(rst);
  wloader->weight_fifo_rdy(weight_loader_weight_fifo_rdy);
  wloader->weight_fifo_wen(weight_loader_weight_fifo_wen);
  wloader->weight_fifo_wdata(weight_loader_weight_fifo_wdata);
  wloader->rf_id_fifo_rdy(weight_loader_rf_id_fifo_rdy);
  wloader->rf_id_fifo_wen(weight_loader_rf_id_fifo_wen);
  wloader->rf_id_fifo_wdata(weight_loader_rf_id_fifo_wdata);
  wloader->rf_addr_fifo_rdy(weight_loader_rf_addr_fifo_rdy);
  wloader->rf_addr_fifo_wen(weight_loader_rf_addr_fifo_wen);
  wloader->rf_addr_fifo_wdata(weight_loader_rf_addr_fifo_wdata);
  wloader->layer_id_fifo_rdy(weight_loader_layer_id_fifo_rdy);
  wloader->layer_id_fifo_wen(weight_loader_layer_id_fifo_wen);
  wloader->layer_id_fifo_wdata(weight_loader_layer_id_fifo_wdata);
  wloader->mvm_id_fifo_rdy(weight_loader_mvm_id_fifo_rdy);
  wloader->mvm_id_fifo_wen(weight_loader_mvm_id_fifo_wen);
  wloader->mvm_id_fifo_wdata(weight_loader_mvm_id_fifo_wdata);

  module_name_str = "inst_loader";
  std::strcpy(module_name, module_name_str.c_str());
  iloader = new inst_loader(module_name);
  iloader->rst(rst);
  iloader->inst_fifo_rdy(inst_loader_inst_fifo_rdy);
  iloader->inst_fifo_wen(inst_loader_inst_fifo_wen);
  iloader->inst_fifo_wdata(inst_loader_inst_fifo_wdata);
  iloader->layer_id_fifo_rdy(inst_loader_layer_id_fifo_rdy);
  iloader->layer_id_fifo_wen(inst_loader_layer_id_fifo_wen);
  iloader->layer_id_fifo_wdata(inst_loader_layer_id_fifo_wdata);
  iloader->mvm_id_fifo_rdy(inst_loader_mvm_id_fifo_rdy);
  iloader->mvm_id_fifo_wen(inst_loader_mvm_id_fifo_wen);
  iloader->mvm_id_fifo_wdata(inst_loader_mvm_id_fifo_wdata);

  radsim_design.BuildDesignContext("mlp.place", "mlp.clks");
  radsim_design.CreateSystemNoCs(rst);
  radsim_design.ConnectModulesToNoC();
}

mlp_top::~mlp_top() {
  for (unsigned int layer_id = 0; layer_id < matrix_vector_engines.size();
       layer_id++) {
    for (unsigned int mvm_id = 0;
         mvm_id < matrix_vector_engines[layer_id].size(); mvm_id++) {
      delete matrix_vector_engines[layer_id][mvm_id];
    }
    for (unsigned int mvm_id = 0;
         mvm_id < rtl_matrix_vector_engines[layer_id].size(); mvm_id++) {
      delete rtl_matrix_vector_engines[layer_id][mvm_id];
    }
  }
  for (unsigned int mvm_id = 0; mvm_id < num_mvms_total[0]; mvm_id++) {
    delete input_dispatchers[mvm_id];
  }
  delete output_collector;
  delete wloader;
  delete iloader;
}