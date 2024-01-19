#pragma once

#include <iostream>
#include <design_context.hpp>
#include <vector>
#include <string>
#include <systemc.h>

class RADSimCluster {
    private:
    public:
        int num_rads;
        std::vector<RADSimDesignContext*> all_rads;
        //std::vector<sc_module*> all_systems; //nth system should be on the nth RAD. also tried std::any, auto
        std::vector<sc_in<bool>> all_systems_in; //nth system should be on the nth RAD
        std::vector<sc_out<bool>> all_systems_out; //nth system should be on the nth RAD
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
        void StoreSystemIn(sc_in<bool> system_in);
        void StoreSystemOut(sc_out<bool> system_out);
};