#include "mlp_system.hpp"

mlp_system::mlp_system(const sc_module_name& name, sc_clock* driver_clk_sig, RADSimDesignContext* radsim_design) :
  sc_module(name) {
  
  // Parse design configuration (number of layers and number of MVMs per layer)
  std::string design_root_dir = radsim_config.GetStringKnobPerRad("radsim_user_design_root_dir", radsim_design->rad_id);
  std::string design_config_filename = design_root_dir + "/compiler/layer_mvm_config";
  std::ifstream design_config_file(design_config_filename);
  if(!design_config_file) {
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

  // Initialize signal vectors
  init_vector<sc_signal<bool>>::init_sc_vector(dispatcher_fifo_rdy_signal, num_mvms[0]);
  init_vector<sc_signal<bool>>::init_sc_vector(dispatcher_fifo_wen_signal, num_mvms[0]);
  init_vector<sc_signal<data_vector<sc_int<32>>>>::init_sc_vector(dispatcher_fifo_wdata_signal, num_mvms[0]);

  // Instantiate driver
  mlp_driver_inst = new mlp_driver("mlp_driver", radsim_design);
  mlp_driver_inst->clk(*driver_clk_sig);
  mlp_driver_inst->rst(rst_sig);
  mlp_driver_inst->dispatcher_fifo_rdy(dispatcher_fifo_rdy_signal);
  mlp_driver_inst->dispatcher_fifo_wen(dispatcher_fifo_wen_signal);
  mlp_driver_inst->dispatcher_fifo_wdata(dispatcher_fifo_wdata_signal);
  mlp_driver_inst->collector_fifo_rdy(collector_fifo_rdy_signal);
  mlp_driver_inst->collector_fifo_ren(collector_fifo_ren_signal);
  mlp_driver_inst->collector_fifo_rdata(collector_fifo_rdata_signal);

  // Instantiate design top-level
  mlp_inst = new mlp_top("mlp_top", radsim_design);
  mlp_inst->rst(rst_sig);
  mlp_inst->dispatcher_fifo_rdy(dispatcher_fifo_rdy_signal);
  mlp_inst->dispatcher_fifo_wen(dispatcher_fifo_wen_signal);
  mlp_inst->dispatcher_fifo_wdata(dispatcher_fifo_wdata_signal);
  mlp_inst->collector_fifo_rdy(collector_fifo_rdy_signal);
  mlp_inst->collector_fifo_ren(collector_fifo_ren_signal);
  mlp_inst->collector_fifo_rdata(collector_fifo_rdata_signal);

  //add _top as dut instance for parent class design_system
  this->design_dut_inst = mlp_inst;
}

mlp_system::~mlp_system() {
  delete mlp_driver_inst;
  delete mlp_inst;
  delete sysclk;
}