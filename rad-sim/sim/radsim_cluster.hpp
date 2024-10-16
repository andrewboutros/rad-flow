#pragma once

#include <iostream>
#include <design_context.hpp>
#include <vector>
#include <string>
#include <systemc.h>
#include <design_system.hpp>

class RADSimCluster {
    private:
    public:
        int num_rads;
        std::vector<RADSimDesignContext*> all_rads;
        std::vector<RADSimDesignSystem*> all_systems;

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
        RADSimDesignContext* CreateNewRAD(unsigned int i); //returns ptr to the newly added RAD
        void SetTopo(inter_rad_topo_type inter_rad_topo);
        void SetConnModel(inter_rad_conn_model_type inter_rad_topo);
        bool AllRADsNotDone();
        void StoreSystem(RADSimDesignSystem* system);
};