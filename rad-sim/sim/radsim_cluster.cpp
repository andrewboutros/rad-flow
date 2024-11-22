#include <radsim_cluster.hpp>

RADSimCluster::RADSimCluster(int num_rads) {
    this->num_rads = num_rads;
    for (int rad_id = 0; rad_id < num_rads; rad_id++) {
        RADSimDesignContext* new_rad = new RADSimDesignContext(rad_id); //pass in unique RAD ID
        all_rads.push_back(new_rad);
    }
    //TODO: use configuration parameters to change topology and connectivity models
    inter_rad_topo = ALL_TO_ALL;
    inter_rad_conn_model = NETWORK;
}

RADSimCluster::~RADSimCluster() { 
    for (int rad_id = 0; rad_id < num_rads; rad_id++) {
        delete all_rads[rad_id]; //free the RADs allocated
    }
}

RADSimDesignContext* 
RADSimCluster::CreateNewRAD(unsigned int rad_id) {
    RADSimDesignContext* new_rad = new RADSimDesignContext(rad_id);
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
    for (int rad_id = 0; rad_id < num_rads; rad_id++) {
        if (!(all_rads[rad_id]->is_rad_done())) {
            return true;
        }
    }
    return false;
}

void
RADSimCluster::StoreSystem(RADSimDesignSystem* system) {
    all_systems.push_back(system);
}