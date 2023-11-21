#pragma once

#include <afifo.hpp>
#include <aximm_interface.hpp>
#include <axis_interface.hpp>
#include <design_context.hpp>
#include <queue>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <sim_utils.hpp>
#include <string>
#include <systemc.h>
#include <vector>

#define NUM_SIG_REQ_IF_PORTS 6


struct axi_port_info {
    /*
        Just the port information required to know where to send packets (used for src/dst ports)
    */
    std::string name; // same as in the <module>.place file
    std::string type; // "aximm", "axis"
    int noc_idx; // Which NoC is this port associated with?
    int router_idx; // Where in the above NoC idx is the router located (coordinate) 
    bool is_master; // master or slave interface
};

struct hw_module {
    std::string inst_name;
    std::string module_name;
    std::vector<axi_port_info> ports;
};

bool ParseTrafficGenModules(std::vector<hw_module> &parsed_modules, std::string &io_filename);
unsigned int count_modules(const std::vector<hw_module>& module_insts, const std::string& mem_req_module_name);


template <typename T>
void axis_bv_to_data_vector(
    sc_bv<AXIS_MAX_DATAW> &bitvector, 
    data_vector<T> &datavector, 
    unsigned int bitwidth, 
    unsigned int num_elements);

template <typename T>
void data_vector_to_bv_axis(
    data_vector<T> &datavector, 
    sc_bv<AXIS_MAX_DATAW> &bitvector, 
    unsigned int bitwidth, 
    unsigned int num_elements);


void data_vector_to_bv_axis(
    data_vector<int16_t> &datavector, 
    sc_bv<AXIS_MAX_DATAW> &bitvector, 
    unsigned int bitwidth, 
    unsigned int num_elements);

template <typename T>
void aximm_bv_to_data_vector(
    sc_bv<AXI4_MAX_DATAW> &bitvector, 
    data_vector<T> &datavector, 
    unsigned int bitwidth, 
    unsigned int num_elements);

template <typename T>
void aximm_vector_to_bv(
    data_vector<T> &datavector, 
    sc_bv<AXI4_MAX_DATAW> &bitvector, 
    unsigned int bitwidth, 
    unsigned int num_elements);

void aximm_bv_to_data_vector(
    sc_bv<AXI4_MAX_DATAW> &bitvector, 
    data_vector<int16_t> &datavector, 
    unsigned int bitwidth, 
    unsigned int num_elements);

// void aximm_bv_to_data_vector(
//     sc_bv<AXI4_MAX_DATAW> &bitvector, 
//     data_vector<int16_t> &datavector, 
//     unsigned int bitwidth, 
//     unsigned int num_elements);
