#pragma once

#include <systemc.h>
#include <iostream>
#include <design_context.hpp>
#include <vector>
#include <string>
#include <radsim_cluster.hpp>

#define DATAW 128

class RADSimInterRad { //: public sc_module {
    private:
        RADSimCluster* cluster;
        //std::vector<sc_signal<bool>> all_signals;
        sc_vector<sc_signal<sc_bv<DATAW>>> all_signals{"all_signals"};
    public:
        RADSimInterRad(RADSimCluster* cluster);
        ~RADSimInterRad();
        void ConnectRadPair(int i, int j);
        /*sc_signal<bool> in_i_out_j;
	    sc_signal<bool> in_j_out_i; */
};