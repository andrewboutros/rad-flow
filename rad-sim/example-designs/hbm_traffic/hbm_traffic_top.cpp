#include <hbm_traffic_top.hpp>



// TODO update this function to support higher level instantiation of these black boxes from the python compiler
// For now it will just grab the number of consumer_producer insts 
/*
bool ParseTrafficGenModules(
        std::vector<hw_module> &modules,
        std::string &io_filename) {

    std::ifstream io_file(io_filename);
    if (!io_file)
        return false;

    std::string module_name;
    std::vector<axi_port_info> ports;
    std::vector<bool> is_masters;
    std::string line;
    while (getline(io_file, line)) {
        std::stringstream ss(line);
        // looking for module & not endmodule
        if (line.find("module") && !line.find("end")){
            // ignore the first space delim value as its the "module" keyword
            ss.ignore(); // looking for space delim and ignoring it up to max size of string stream
            ss >> module_name;
        } else if (line.find("endmodule")) {
            hw_module module_inst("black_box_0_inst", module_name, ports);       
            modules.push_back(module_inst);
            ports.clear(); // clear out ports vector for next module
            break; // TEST TODO remove, just creating the first module and no others, ext mem manually created
        } else {
            // parsing the port info, we only care about the port names 
            axi_port_info port;
            bool is_master;
            ss >> port.name;
            ss.ignore(); // this one is for noc idx (not using at the moment)
            ss.ignore(); // noc_loc -> where this port is on the noc (specified by above noc_idx) placement
            ss >> port.type;
            ss >> port.is_master;
        }
    }
}
*/

hbm_traffic_top::hbm_traffic_top(const sc_module_name &name) : sc_module(name) {

    unsigned int line_bitwidth = 512;
    unsigned int element_bitwidth = 16;
    std::vector<unsigned int> mem_channels = {1, 1, 8, 8};
    unsigned int embedding_lookup_fifos_depth = 64;
    unsigned int feature_interaction_fifos_depth = 64;
    unsigned int num_mem_controllers =
        radsim_config.GetIntKnob("dram_num_controllers");
    assert(num_mem_controllers == mem_channels.size());
    unsigned int total_mem_channels = 0;
    for (auto &num_channels : mem_channels) {
        total_mem_channels += num_channels;
    }

    unsigned int mem_req_fifos_depth = 8;


    std::string module_name_str;
    char module_name[50]; // 25

    // Parse the traffic gen configuration
    std::string design_root_dir = radsim_config.GetStringKnob("radsim_user_design_root_dir");

    std::vector<hw_module> modules;
    std::string mod_insts_fname = design_root_dir + "/compiler/traffic_gen/traffic_gen.cfg";
    ParseTrafficGenModules(modules, mod_insts_fname);

    // now we can find the number of black box modules parsed from module isnts
    unsigned int num_mem_req_modules = std::count_if(modules.begin(), modules.end(),
                              [](const hw_module& module) {
                                  return module.module_name == "black_box";
                              });

    // Init sc_vectors for black box mem request interface
    
    mem_req_valids.init(num_mem_req_modules);
    mem_req_readys.init(num_mem_req_modules);
    wr_ens.init(num_mem_req_modules);
    
    black_boxes.resize(num_mem_req_modules);

    // Init sc_vectors required for converting data_vectors to sc_signals, TODO see if this is a hack
    // Conversion of these signals is done in Assign()
    _target_channels.init(num_mem_req_modules);
    _target_addresses.init(num_mem_req_modules);
    _wr_datas.init(num_mem_req_modules);
    _src_ports.init(num_mem_req_modules);
    _dst_ports.init(num_mem_req_modules);

    // Verif Ports
    _rd_req_data.init(num_mem_req_modules);
    _wr_req_data.init(num_mem_req_modules);


    
    // Instantiate mem req modules (black boxes)
    int i = 0;
    unsigned int ctrl_id = 0;
    unsigned int ch_id = 0;
    ext_mem.resize(num_mem_controllers);
    mem_clks.resize(num_mem_controllers);
    for (auto &module: modules){
        // Consumer producer modules
        if (module.module_name == "black_box") {
            std::strcpy(module_name, module.inst_name.c_str());
            black_boxes[i] = new black_box(module_name, module, line_bitwidth, element_bitwidth, mem_req_fifos_depth);
            black_boxes[i]->rst(rst);
            black_boxes[i]->wr_en(wr_ens[i]);
            black_boxes[i]->mem_req_valid(mem_req_valids[i]);
            black_boxes[i]->mem_req_ready(mem_req_readys[i]);
            black_boxes[i]->target_channel(_target_channels[i]);
            black_boxes[i]->target_address(_target_addresses[i]);
            black_boxes[i]->src_port(_src_ports[i]);
            black_boxes[i]->dst_port(_dst_ports[i]);
            black_boxes[i]->wr_data(_wr_datas[i]);
            black_boxes[i]->rd_req_data(_rd_req_data[i]);
            black_boxes[i]->wr_req_data(_wr_req_data[i]);
            black_boxes[i]->rd_req_data_rdy(rd_req_data_rdy);

        } else if (module.module_name == "ext_mem_ctrl") {
            // Ext Mem Controllers TODO 
            std::string mem_content_init_prefix = radsim_config.GetStringKnob("radsim_user_design_root_dir") + "/compiler/ext_mem_init/channel_";
            double mem_clk_period = radsim_config.GetDoubleVectorKnob("dram_clk_periods", ctrl_id);
            module_name_str = module.inst_name + "_clk"; //"ext_mem_" + std::to_string(ctrl_id) + "_isnt_" + "_clk";
            std::strcpy(module_name, module_name_str.c_str());
            mem_clks[ctrl_id] = new sc_clock(module_name, mem_clk_period, SC_NS);
            module_name_str = module.inst_name; // "ext_mem_" + std::to_string(ctrl_id) + "_isnt";
            std::strcpy(module_name, module_name_str.c_str());
            // if ext mem has 8 channels and the mem content init channel is set to 0 then instantiating it will use channels 0-7
            std::string mem_content_init = mem_content_init_prefix + std::to_string(ch_id);
            ch_id += mem_channels[ctrl_id]; // increment channel count by the num channels used by cur ctrler
            ext_mem[ctrl_id] = new mem_controller(module_name, module, ctrl_id, mem_content_init);
            ext_mem[ctrl_id]->mem_clk(*mem_clks[ctrl_id]);
            ext_mem[ctrl_id]->rst(rst);
            ctrl_id++;
        }
        i++;
    }


    // for (int i=0; i < num_mem_req_modules; i++){
    //     if (modules[i].module_name == "black_box") {
    //         std::string module_name_str = "black_box_" + std::to_string(i) + "_inst";
    //         std::strcpy(module_name, module_name_str.c_str());
    //         black_boxes[i] = new black_box(module_name, line_bitwidth, element_bitwidth, mem_req_fifos_depth);

    //     }
    //     // Ext Mem Controllers TODO
        

    // }



    /*
        // Parse MVM configuration
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
    */

    // Instantiate Embedding Lookup Module
    /*
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
    */

    // Instantiate Black Box Modules 


    // hack to get around this top level recieved responses signal
    // std::vector<sc_out<unsigned int>> bb_received_responses;

    // bb_received_responses(received_responses); //connect to top level


    /*
        uint num_mem_channels = 18;
        // bb_received_responses.resize(num_mem_channels);
        std::string black_box_names = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        black_boxes.resize(num_mem_channels);
        assert (num_mem_channels <= black_box_names.size());
        for (uint ch_id = 0; ch_id < num_mem_channels; ch_id++){
            std::stringstream ss;  
            ss << "black_box_" << black_box_names[ch_id] << "_inst";
            module_name_str = ss.str();
            ss.str(std::string()); //clears string stream
            std::strcpy(module_name, module_name_str.c_str());
            black_boxes[ch_id] = new black_box(module_name, black_box_names[ch_id], line_bitwidth, element_bitwidth, embedding_lookup_fifos_depth);
            black_boxes[ch_id]->rst(rst);

            // black_boxes[ch_id]->target_channel(target_channel[ch_id]);
            // black_boxes[ch_id]->target_address(target_address[ch_id]);
            // black_boxes[ch_id]->write_en(write_en[ch_id]);
            // black_boxes[ch_id]->mem_req_valid(mem_req_valid[ch_id]);
            // black_boxes[ch_id]->mem_req_ready(mem_req_ready[ch_id]);


            // black_boxes[ch_id]->received_responses(bb_received_responses[ch_id]);
            // if (ch_id == 0){
            //   black_boxes[ch_id]->received_responses(bb_received_response);
            // }

            // if (ch_id == 0) {
            //   black_boxes[ch_id]->received_responses(received_responses);
            // }
        }
    */

    // Connect just bb module A to the top level
    // sc_out<unsigned int> bb_received_response;
    // bb_received_response(received_responses);
    // bb_received_response.write(0);



    // Instantiate Feature Interaction Module
    
    // module_name_str = "feature_interaction_inst";
    // std::strcpy(module_name, module_name_str.c_str());
    // std::string feature_interaction_inst_file =
    //     radsim_config.GetStringKnob("radsim_user_design_root_dir") +
    //     "/compiler/instructions/feature_interaction.inst";
    // feature_interaction_inst = new custom_feature_interaction(
    //     module_name, line_bitwidth, element_bitwidth, total_mem_channels,
    //     feature_interaction_fifos_depth, num_mvms[0],
    //     feature_interaction_inst_file);
    // feature_interaction_inst->rst(rst);
    // feature_interaction_inst->received_responses(received_responses);


    // Instantiate MVM Engines

    /*
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
    */

    // Instantiate Output Collector
    module_name_str = "output_collector_0_inst";
    std::strcpy(module_name, module_name_str.c_str());
    output_collector = new collector(module_name);
    output_collector->rst(rst);
    output_collector->data_fifo_rdy(collector_fifo_rdy);
    output_collector->data_fifo_ren(collector_fifo_ren);
    output_collector->data_fifo_rdata(collector_fifo_rdata);

    // Init external memory controllers
    /*
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
    */

    // set clk and reset signal for the top level module
    
    SC_METHOD(Assign);
    sensitive << rst << target_channels << target_addresses << wr_datas << src_ports << dst_ports;
    // Verif ports
    // sensitive << rd_req_data << wr_req_data;
    for (unsigned int i = 0; i < mem_req_valids.size(); i++) {
        sensitive << mem_req_readys[i] << mem_req_valids[i];
        sensitive << _rd_req_data[i] << _wr_req_data[i];
    }
    

    radsim_design.BuildDesignContext("hbm_traffic.place", "hbm_traffic.clks");
    radsim_design.CreateSystemNoCs(rst);
    radsim_design.ConnectModulesToNoC();
}


void hbm_traffic_top::Assign(){
    // All module instantiations will deal with other signals, this is just for the conversion between data_vectors to sc_vectors, again TODO see if this is a hack, I think it is....
    if (rst) {
        for (unsigned int i = 0; i < mem_req_valids.size(); i++) {
            _target_channels[i].write(0);
            _target_addresses[i].write(0);
            _wr_datas[i].write(0);
            _src_ports[i].write(0);
            _dst_ports[i].write(0);
            // Verif
            // _rd_req_data[i].write(0);
            // _wr_req_data[i].write(0);
            rd_req_data.write(0);
            wr_req_data.write(0);
        }
    } else {
        for (unsigned int i = 0; i < mem_req_valids.size(); i++) {
            if (mem_req_readys[i].read() && mem_req_valids[i].read()) {
                // convert the data_vectors to sc_vectors
                data_vector<unsigned int> tmp_target_channels = target_channels.read();
                data_vector<uint64_t> tmp_target_addresses = target_addresses.read();
                data_vector<size_t> tmp_wr_datas = wr_datas.read();
                data_vector<uint64_t> tmp_src_ports = src_ports.read();
                data_vector<uint64_t> tmp_dst_ports = dst_ports.read();


                _target_channels[i].write(tmp_target_channels[i]);
                _target_addresses[i].write(tmp_target_addresses[i]);
                _wr_datas[i].write(tmp_wr_datas[i]);
                _src_ports[i].write(tmp_src_ports[i]);
                _dst_ports[i].write(tmp_dst_ports[i]);
            }
            // Verif Ports
            data_vector<uint64_t> tmp_rd_req_data(_rd_req_data);
            data_vector<uint64_t> tmp_wr_req_data(_wr_req_data);
            // Verif Ports
            rd_req_data.write(tmp_rd_req_data);
            wr_req_data.write(tmp_wr_req_data);
        }
    }
}


hbm_traffic_top::~hbm_traffic_top() {
  delete embedding_lookup_inst;
  for (auto &bb : black_boxes) {
    delete bb;
  }
  for (auto &ctrlr : ext_mem)
    delete ctrlr;
  delete output_collector;
  for (unsigned int l = 0; l < mvms.size(); l++) {
    for (auto &mvm : mvms[l]) {
      delete mvm;
    }
  }
}