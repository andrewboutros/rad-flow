#pragma once

#include <systemc.h>
#include <iostream>
#include <design_context.hpp>
#include <vector>
#include <string>
#include <radsim_cluster.hpp>
#include <radsim_config.hpp>

#define DATAW 128
#define NUM_SLOTS 2 //number of fifo slots

class RADSimInterRad : public sc_module {
    private:
        RADSimCluster* cluster;
        //std::vector<sc_signal<bool>> all_signals;
        sc_vector<sc_signal<sc_bv<DATAW>>> all_signals{"all_signals"};
        //sc_fifo<sc_bv<DATAW>> data_in_rad1 = sc_fifo<sc_bv<DATAW>>(2); //2 slots for now
        //sc_vector<sc_fifo<sc_bv<DATAW>>> switch_port_fifos{"switch_port_fifos"};
    public:
        sc_in<bool> clk;
        std::vector<sc_fifo<sc_bv<DATAW>>*> fifos;
        RADSimInterRad(const sc_module_name &name, sc_clock *inter_rad_clk, RADSimCluster* cluster);
        ~RADSimInterRad();
        void ConnectRadPair(int i, int j);
        /*sc_signal<bool> in_i_out_j;
	    sc_signal<bool> in_j_out_i; */
        void writeFifo();
        void readFifo();
        SC_HAS_PROCESS(RADSimInterRad);
};