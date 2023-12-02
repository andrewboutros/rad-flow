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
#include <traffic_gen.hpp>

#define NUM_MEM_REQ_FIFOS 6

#define NUM_SIG_REQ_IF_PARALLEL_PORTS 1


enum aximm_wr_state_t {
    AXIMM_WR_IDLE, // Waiting for a write request
    AXIMM_WR_ADDR, // Sending ADDR + CTRL INFO
    AXIMM_WR_DATA_IPR, // Sending Data flits 1 -> N-1
    AXIMM_WR_DATA_LAST, // Sending Data flit N
    AXIMM_WR_COMMIT, // Waiting for response from memory
};

enum aximm_rd_state_t {
    AXIMM_RD_IDLE, // Waiting for a write request
    AXIMM_RD_ADDR, // Sending ADDR + CTRL INFO
    AXIMM_RD_COMMIT, // Read Request Accepted, can pop inst fifo
};

/*
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
*/



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


    sc_signal<unsigned int> _wr_req_id_count; // Counters for transaction IDs
    sc_signal<unsigned int> _wr_data_id_count; // Counters for transaction IDs
    sc_signal<unsigned int> _rd_req_id_count; // Counters for transaction IDs
    
    sc_signal<unsigned int> _rd_data_id_count; // Counters for transaction IDs
    sc_signal<unsigned int> _wr_resp_id_count; // Counters for transaction IDs



    sc_vector<sc_signal<bool>> _mem_req_ififos_full; // Signals flagging FIFOs are full
    sc_vector<sc_signal<bool>> _mem_req_ififos_empty; // Signals flagging FIFOs are full

    std::queue<uint64_t> _target_addr_fifo;
    std::queue<unsigned int> _target_channel_fifo;
    std::queue<size_t> _wr_data_fifo;
    std::queue<bool> _wr_en_fifo;
    std::queue<uint64_t> _src_port_fifo;
    std::queue<uint64_t> _dst_port_fifo;

    // starting with 1 input and 1 output FIFO
    std::queue<data_vector<int16_t>> _rd_data_input_fifo; // Input FIFO
    sc_signal<bool> _rd_data_ififo_full;               // Signals FIFO full
    sc_signal<bool> _rd_data_ififo_empty;              // Signals iFIFO empty

    std::queue<data_vector<int16_t>> _output_fifo; // Output FIFO
    sc_signal<bool> _ofifo_full;               // Signals FIFO full
    sc_signal<bool> _ofifo_empty;              // Signals oFIFO empty
    
    unsigned int _num_mem_channels; // No. of memory channels = # of AXI-MM routers
    unsigned int _num_output_channels; // No. of output AXI-S routers
    unsigned int _dataw;            // Data interface bitwidth

    // info for debug to verify functionality
    unsigned int _num_received_responses; // Mem req resp recieved
    unsigned int _num_expected_responses; // Mem req resp expected

    sc_signal<bool> _aximm_wr_ctrl_sent;
    sc_signal<unsigned int> _aximm_wr_num_sent_flits;
    sc_signal<aximm_wr_state_t> _aximm_wr_state; 
    sc_signal<bool> _aximm_wr_tx_done;
    sc_signal<bool> _aximm_rd_tx_done;



    ofstream *_debug_black_box_out;

public:

    // Interface to driver logic
    sc_in<bool> rst;

    // Interface to the NoC
    // vectors of all possible AXI interfaces which will be instantiated and connected via module_conf
    // TODO figure out how to make the below work, there's some weirdness to do with vectors of structs containing systemc ports
    /*
        std::vector<aximm_master_port> aximm_master_ifs;
        std::vector<aximm_slave_port> aximm_slave_ifs;
        std::vector<axis_master_port> axis_master_ifs;
        std::vector<axis_slave_port> axis_slave_ifs;
    */

    
    // Manually defined axi interfaces
    
    
    aximm_master_port aximm_interface;
    axis_master_port axis_interface;
    
    

    // interface from testbench to generate memory read requests
    sc_in<bool> wr_en; // 0 = read, 1 = write
    sc_in<bool> mem_req_valid;
    sc_out<bool> mem_req_ready;
    
    // None of these are vectors, we can only issues a single req at a time
    sc_in<size_t> wr_data;
    sc_in<unsigned int> target_channel;
    sc_in<uint64_t> target_address;
    sc_in<uint64_t> src_port;
    sc_in<uint64_t> dst_port;

    // Verif Ports
    sc_out<uint64_t> rd_req_data;
    sc_out<uint64_t> wr_req_data;
    sc_out<bool> rd_req_data_rdy;
    

    // Module

    
    // void data_vector_to_bv_axis(
    //     data_vector<int16_t> &datavector, sc_bv<AXIS_MAX_DATAW> &bitvector,
    //     unsigned int num_elements);

    // void bv_aximm_to_data_vector(
    //     sc_bv<AXI4_MAX_DATAW> &bitvector, data_vector<int16_t> &datavector,
    //     unsigned int num_elements);
    

    black_box(  const sc_module_name &name,
                hw_module &module_conf,
                unsigned int line_dataw,
                unsigned int element_bitwidth,
                // unsigned int num_mem_channels,
                // unsigned int num_output_channels,
                unsigned int fifos_depth);
    ~black_box();
    void Assign(); // Combinational logic process
    void Tick();   // Sequential logic process
    
    SC_HAS_PROCESS(black_box);
    void RegisterModuleInfo(hw_module &module_conf);
    void RegisterModuleInfo(); // To deal with virtual function bs


};


