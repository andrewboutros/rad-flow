#pragma once

#include <iostream>
#include <design_context.hpp>
#include <vector>
#include <string>
#include <systemc.h>
#include <design_system.hpp>

//Represents a cluster of one or multiple RAD devices
//Stores pointers to objects representing the RADs and the designs on each RAD
//Contains support for future development of new topologies for inter-RAD connections
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
            WIRE = 0, //Direct wire-based connection between RADs. This option has been deprecated.
            NETWORK = 1 //Current approach using bandwidth and latency constraints from the user for inter-RAD communication.
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