#include <rtl_mvm.hpp>

rtl_mvm::rtl_mvm(const sc_module_name &name) : RADSimModule(name) {
	char vrtl_mvm_name[25];
	std::string vrtl_mvm_name_str = std::string(name) + "_vmvm";
	std::strcpy(vrtl_mvm_name, vrtl_mvm_name_str.c_str());

	vrtl_mvm = new Vrtl_mvm{vrtl_mvm_name};
	vrtl_mvm->clk(clk);
	vrtl_mvm->rst(rst);
	vrtl_mvm->axis_rx_tvalid(axis_rx.tvalid);
	vrtl_mvm->axis_rx_tdata(axis_rx.tdata);
	vrtl_mvm->axis_rx_tstrb(axis_rx.tstrb);
	vrtl_mvm->axis_rx_tkeep(axis_rx.tkeep);
	vrtl_mvm->axis_rx_tid(axis_rx.tid);
	vrtl_mvm->axis_rx_tdest(axis_rx.tdest);
	vrtl_mvm->axis_rx_tuser(axis_rx.tuser);
	vrtl_mvm->axis_rx_tlast(axis_rx.tlast);
	vrtl_mvm->axis_rx_tready(axis_rx.tready);
	vrtl_mvm->axis_tx_tvalid(axis_tx.tvalid);
	vrtl_mvm->axis_tx_tdata(axis_tx.tdata);
	vrtl_mvm->axis_tx_tstrb(axis_tx.tstrb);
	vrtl_mvm->axis_tx_tkeep(axis_tx.tkeep);
	vrtl_mvm->axis_tx_tid(axis_tx.tid);
	vrtl_mvm->axis_tx_tdest(axis_tx.tdest);
	vrtl_mvm->axis_tx_tuser(axis_tx.tuser);
	vrtl_mvm->axis_tx_tlast(axis_tx.tlast);
	vrtl_mvm->axis_tx_tready(axis_tx.tready);

	this->RegisterModuleInfo();
}

rtl_mvm::~rtl_mvm() {
	vrtl_mvm->final();
	delete vrtl_mvm;
}

void rtl_mvm::RegisterModuleInfo() {
	std::string port_name;
	_num_noc_axis_slave_ports = 0;
	_num_noc_axis_master_ports = 0;
	_num_noc_aximm_slave_ports = 0;
	_num_noc_aximm_master_ports = 0;

	port_name = module_name + ".axis_rx";
	RegisterAxisSlavePort(port_name, &axis_rx, 512, 0);

	port_name = module_name + ".axis_tx";
	RegisterAxisMasterPort(port_name, &axis_tx, 512, 0);

}
