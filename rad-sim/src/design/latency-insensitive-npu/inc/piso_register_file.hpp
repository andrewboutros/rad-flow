#pragma once

#include <systemc.h>
#include <sstream>
#include <params.hpp>
#include <sim_utils.hpp>
#include <register_file.hpp>

template <class dtype>
class piso_register_file: public sc_module {
private:
    unsigned int register_file_id;
    unsigned int full_width;
    unsigned int num_elements_per_word;
    std::vector<std::vector< std::vector<dtype>>> mem;

    sc_vector <sc_signal<data_vector<dtype>>> rdata_pipeline;
    sc_vector <sc_signal<data_vector<dtype>>> wdata_pipeline;
    sc_vector <sc_signal<uint32_t>> waddr_pipeline;
    sc_vector <sc_signal<bool>> wen_pipeline;

public:
    // Register file inputs
    sc_in <bool> clk;
    sc_in <bool> rst;
    sc_in <uint32_t> raddr;
    sc_in <uint8_t> rd_id;
    sc_in <data_vector<dtype>> wdata;
    sc_in <uint32_t> waddr;
    sc_in <bool> wen;
    sc_in <bool> clk_en;

    // Register file outputs
    sc_out <data_vector<dtype>> rdata;

    // DPE constructor and destructor
    piso_register_file(const sc_module_name& name, unsigned int id, unsigned int depth, unsigned int width, std::string& init_file);
    ~piso_register_file();

    void Tick();
    void Assign();

    SC_HAS_PROCESS(piso_register_file);
};