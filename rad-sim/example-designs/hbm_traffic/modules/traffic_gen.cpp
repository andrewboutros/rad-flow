#include <traffic_gen.hpp>



// TODO update this function to support higher level instantiation of these black boxes from the python compiler
// For now it will just grab the number of consumer_producer insts 
bool ParseTrafficGenModules(
        std::vector<hw_module> &parsed_modules,
        std::string &io_filename) {

    std::ifstream io_file(io_filename);
    if (!io_file)
        return false;


    hw_module cur_module;
    // std::vector<axi_port_info> cur_ports;
    std::string line;
    while (getline(io_file, line)) {
        std::stringstream ss(line);
        // looking for module & not endmodule
        if (line.find("module") != std::string::npos && !(line.find("end") != std::string::npos)){
            // std::string module_inst;
            std::string module_keyword;
            // ignore the first space delim value as its the "module" keyword
            ss >> module_keyword; // we don't use this just want to ignore the first one
            ss >> cur_module.module_name;
        }
        else if (line.find("endmodule") != std::string::npos){
            // Module finished
            parsed_modules.push_back(cur_module);
        }
        else {
            // Parse the port definitions
            axi_port_info cur_port;
            ss >> cur_port.name;
            ss >> cur_port.noc_idx;
            ss >> cur_port.router_idx;
            ss >> cur_port.type;
            ss >> cur_port.is_master;
            cur_module.ports.push_back(cur_port);
        }
    }
}


// Member function to count occurrences of a specific module name
unsigned int count_modules(const std::vector<hw_module>& module_insts, const std::string& mem_req_module_name) {
    return std::count_if(module_insts.begin(), module_insts.end(),
                        [mem_req_module_name](const hw_module& cur_hw_module) {
                            return cur_hw_module.module_name == mem_req_module_name;
                        });
}


template <typename T>
void axis_bv_to_data_vector(
    sc_bv<AXIS_MAX_DATAW> &bitvector, 
    data_vector<T> &datavector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * bitwidth;
    end_idx = (e + 1) * bitwidth;
    datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
  }
}

template <typename T>
void aximm_bv_to_data_vector(
    sc_bv<AXI4_MAX_DATAW> &bitvector, 
    data_vector<T> &datavector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * bitwidth;
    end_idx = (e + 1) * bitwidth;
    datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
  }
}

void aximm_bv_to_data_vector(
    sc_bv<AXI4_MAX_DATAW> &bitvector, 
    data_vector<int16_t> &datavector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * bitwidth;
    end_idx = (e + 1) * bitwidth;
    datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
  }
}

template <typename T>
void data_vector_to_bv_axis( 
    data_vector<T> &datavector, 
    sc_bv<AXIS_MAX_DATAW> &bitvector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {
    unsigned int start_idx, end_idx;
    for (unsigned int e = 0; e < num_elements; e++) {
        start_idx = e * bitwidth;
        end_idx = (e + 1) * bitwidth;
        bitvector.range(end_idx - 1, start_idx) = datavector[e];
    }
}

template <typename T>
void data_vector_to_bv_aximm( 
    data_vector<T> &datavector, 
    sc_bv<AXI4_MAX_DATAW> &bitvector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {
    unsigned int start_idx, end_idx;
    for (unsigned int e = 0; e < num_elements; e++) {
        start_idx = e * bitwidth;
        end_idx = (e + 1) * bitwidth;
        bitvector.range(end_idx - 1, start_idx) = datavector[e];
    }
}

void data_vector_to_bv_axis( 
    data_vector<int16_t> &datavector, 
    sc_bv<AXIS_MAX_DATAW> &bitvector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {
    unsigned int start_idx, end_idx;
    for (unsigned int e = 0; e < num_elements; e++) {
        start_idx = e * bitwidth;
        end_idx = (e + 1) * bitwidth;
        bitvector.range(end_idx - 1, start_idx) = datavector[e];
    }
}