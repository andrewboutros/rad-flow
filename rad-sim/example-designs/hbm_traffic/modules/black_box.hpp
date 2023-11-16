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




class black_box : public RADSimModule {
private:
    // char _bb_tag; // Tag for black box

    unsigned int _fifos_depth;                 // Depth of input/output FIFOs
    
    unsigned int _num_input_elements;
    unsigned int _num_output_elements;
    unsigned int _bitwidth;                    // Bitwidth of single element of data from AXI

    
    // FIFOs to store memory request inputs from testbench 
    // std::queue<data_vector<unsigned int>> _target_channel_fifo;
    // std::queue<data_vector<uint64_t>> _target_address_fifo;
    // std::queue<bool> _write_en_fifo;
    // sc_signal<bool> _mem_req_inst_fifo_full;
    // sc_signal<bool> _mem_req_inst_fifo_empty;


    

    // starting with 1 input and 1 output FIFO
    std::queue<data_vector<int16_t>> _input_fifo; // Input FIFO
    sc_signal<bool> _ififo_full;               // Signals FIFO full
    sc_signal<bool> _ififo_empty;              // Signals iFIFO empty

    std::queue<data_vector<int16_t>> _output_fifo; // Output FIFO
    sc_signal<bool> _ofifo_full;               // Signals FIFO full
    sc_signal<bool> _ofifo_empty;              // Signals oFIFO empty
    
    unsigned int _num_mem_channels; // No. of memory channels = # of AXI-MM routers
    unsigned int _num_output_channels; // No. of output AXI-S routers
    unsigned int _dataw;            // Data interface bitwidth

    // info for debug to verify functionality
    unsigned int _num_received_responses; // Mem req resp recieved
    unsigned int _num_expected_responses; // Mem req resp expected
    ofstream *_debug_black_box_out;

public:

    // Interface to driver logic
    sc_in<bool> rst;

    // Interface to the NoC
    aximm_master_port aximm_interface;
    axis_master_port axis_interface;

    // interface from testbench to generate memory read requests
    // sc_in<data_vector<unsigned int>> target_channel;
    // sc_in<data_vector<uint64_t>> target_address;
    // sc_in<bool> write_en; // 0 = read, 1 = write
    // sc_in<bool> mem_req_valid;
    // sc_out<bool> mem_req_ready;


    
    void data_vector_to_bv_axis(
        data_vector<int16_t> &datavector, sc_bv<AXIS_MAX_DATAW> &bitvector,
        unsigned int num_elements);

    void bv_aximm_to_data_vector(
        sc_bv<AXI4_MAX_DATAW> &bitvector, data_vector<int16_t> &datavector,
        unsigned int num_elements);
    

    black_box(  const sc_module_name &name,
                unsigned int line_dataw,
                unsigned int element_bitwidth,
                // unsigned int num_mem_channels,
                // unsigned int num_output_channels,
                unsigned int fifos_depth);
    ~black_box();
    void Assign(); // Combinational logic process
    void Tick();   // Sequential logic process
    
    SC_HAS_PROCESS(black_box);
    void RegisterModuleInfo();

};


