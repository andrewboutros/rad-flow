#include <radsim_cluster.hpp>

RADSimCluster::RADSimCluster(int num_rads) {
    this->num_rads = num_rads;
    for (unsigned int i = 0; i < num_rads; i++) {
        RADSimDesignContext* new_rad = new RADSimDesignContext(i); //pass in unique RAD ID
        all_rads.push_back(new_rad);
    }
    //TODO: use configuration parameters to change topology and connectivity models
    inter_rad_topo = ALL_TO_ALL;
    inter_rad_conn_model = NETWORK;
}

RADSimCluster::~RADSimCluster() { 
    for (int i = 0; i < num_rads; i++) {
        delete all_rads[i]; //free the RADs allocated
    }
}

RADSimDesignContext* 
RADSimCluster::CreateNewRAD(unsigned int i) {
    RADSimDesignContext* new_rad = new RADSimDesignContext(i);
    num_rads++;
    all_rads.push_back(new_rad);
    return new_rad;
}

void
RADSimCluster::SetTopo(inter_rad_topo_type inter_rad_topo) {
    this->inter_rad_topo = inter_rad_topo;
}

void
RADSimCluster::SetConnModel(inter_rad_conn_model_type inter_rad_conn_model) {
    this->inter_rad_conn_model = inter_rad_conn_model;
}

bool
RADSimCluster::AllRADsNotDone() {
    for (int i = 0; i < num_rads; i++) {
        if (!(all_rads[i]->is_rad_done())) {
            return true;
        }
    }
    return false;
}

void
RADSimCluster::StoreSystem(RADSimDesignSystem* system) {
    all_systems.push_back(system);
}