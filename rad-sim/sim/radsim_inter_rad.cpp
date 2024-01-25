#include <radsim_inter_rad.hpp>

RADSimInterRad::RADSimInterRad(RADSimCluster* cluster) {
    this->cluster = cluster;
}

RADSimInterRad::~RADSimInterRad() {}

/*void
RADSimInterRad::ConnectRadPair(int i, int j) {
    sc_signal<bool> in_i_out_j;
	sc_signal<bool> in_j_out_i;

    all_signals.push_back(in_i_out_j);
    all_signals.push_back(in_j_out_i);

    cluster->all_systems_in[i](in_i_out_j);
    cluster->all_systems_out[i](in_j_out_i);
	cluster->all_systems_in[j](in_j_out_i);
	cluster->all_systems_out[j](in_i_out_j);
}*/