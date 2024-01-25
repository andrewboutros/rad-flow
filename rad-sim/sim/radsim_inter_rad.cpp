#include <radsim_inter_rad.hpp>

RADSimInterRad::RADSimInterRad(RADSimCluster* cluster) {
    this->cluster = cluster;
    all_signals.init(cluster->num_rads);
}

RADSimInterRad::~RADSimInterRad() {}

void
RADSimInterRad::ConnectRadPair(int i, int j) {
    /*sc_signal<bool> in_i_out_j;
	sc_signal<bool> in_j_out_i;

    this->all_signals.push_back(&in_i_out_j);
    this->all_signals.push_back(&in_j_out_i);

    cluster->all_systems[i]->design_dut_inst->portal_in(in_i_out_j);
    cluster->all_systems[i]->design_dut_inst->portal_out(in_j_out_i);
	cluster->all_systems[j]->design_dut_inst->portal_in(in_j_out_i);
	cluster->all_systems[j]->design_dut_inst->portal_out(in_i_out_j); */

    cluster->all_systems[i]->design_dut_inst->portal_in(all_signals[0]);
    cluster->all_systems[i]->design_dut_inst->portal_out(all_signals[1]);
	cluster->all_systems[j]->design_dut_inst->portal_in(all_signals[1]);
	cluster->all_systems[j]->design_dut_inst->portal_out(all_signals[0]);
}