#pragma once

#include <systemc.h>
#include <iostream>
#include <design_context.hpp>
#include <vector>
#include <string>
#include <radsim_cluster.hpp>

class RADSimInterRad {
    private:
        RADSimCluster* cluster;
        //std::vector<sc_signal<bool>> all_signals;
    public:
        RADSimInterRad(RADSimCluster* cluster);
        ~RADSimInterRad();
        //void ConnectRadPair(int i, int j);
};