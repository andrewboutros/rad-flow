#pragma once

#include <iostream>
#include <design_context.hpp>
#include <vector>
#include <string>

class RADSimCluster {
    private:
    public:
        int num_rads;
        std::vector<RADSimDesignContext*> all_rads;
        enum inter_rad_topo_type { 
            ALL_TO_ALL = 0,
            SWITCH = 1,
            RING = 2
        };
        enum inter_rad_conn_model_type {
            WIRE = 0,
            STAT = 1,
            SIM = 2
        };
        inter_rad_topo_type inter_rad_topo;
        inter_rad_conn_model_type inter_rad_conn_model;

        RADSimCluster(int num_rads);
        ~RADSimCluster();
        RADSimDesignContext* CreateNewRAD(); //returns ptr to the newly added RAD
        void SetTopo(inter_rad_topo_type inter_rad_topo);
        void SetConnModel(inter_rad_conn_model_type inter_rad_topo);
        bool AllRADsNotDone();
};