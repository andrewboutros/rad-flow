#include "inst_loader.hpp"

inst_loader::inst_loader(const sc_module_name &name)
    : RADSimModule(name), 
      rst("rst"), 
      inst_fifo_rdy("inst_fifo_rdy"),
      inst_fifo_wen("inst_fifo_wen"), 
      inst_fifo_wdata("inst_fifo_wdata"),
      layer_id_fifo_rdy("layer_id_fifo_rdy"),
      layer_id_fifo_wen("layer_id_fifo_wen"),
      layer_id_fifo_wdata("layer_id_fifo_wdata"),
      mvm_id_fifo_rdy("mvm_id_fifo_rdy"),
      mvm_id_fifo_wen("mvm_id_fifo_wen"),
      mvm_id_fifo_wdata("mvm_id_fifo_wdata") {

  module_name = name;

  char fifo_name[25];
  std::string fifo_name_str;

  fifo_name_str = "inst_loader_inst_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  inst_fifo = new fifo<mvm_inst>(fifo_name, FIFOD, FIFOD - 1);
  inst_fifo->clk(clk);
  inst_fifo->rst(rst);
  inst_fifo->push(inst_fifo_wen);
  inst_fifo->pop(inst_fifo_pop);
  inst_fifo->idata(inst_fifo_wdata);
  inst_fifo->full(inst_fifo_full);
  inst_fifo->almost_full(inst_fifo_almost_full);
  inst_fifo->empty(inst_fifo_empty);
  inst_fifo->odata(inst_fifo_odata);

  fifo_name_str = "inst_loader_layer_id_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  layer_id_fifo = new fifo<uint8_t>(fifo_name, FIFOD, FIFOD - 1);
  layer_id_fifo->clk(clk);
  layer_id_fifo->rst(rst);
  layer_id_fifo->push(layer_id_fifo_wen);
  layer_id_fifo->pop(layer_id_fifo_pop);
  layer_id_fifo->idata(layer_id_fifo_wdata);
  layer_id_fifo->full(layer_id_fifo_full);
  layer_id_fifo->almost_full(layer_id_fifo_almost_full);
  layer_id_fifo->empty(layer_id_fifo_empty);
  layer_id_fifo->odata(layer_id_fifo_odata);

  fifo_name_str = "inst_loader_mvm_id_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  mvm_id_fifo = new fifo<uint8_t>(fifo_name, FIFOD, FIFOD - 1);
  mvm_id_fifo->clk(clk);
  mvm_id_fifo->rst(rst);
  mvm_id_fifo->push(mvm_id_fifo_wen);
  mvm_id_fifo->pop(mvm_id_fifo_pop);
  mvm_id_fifo->idata(mvm_id_fifo_wdata);
  mvm_id_fifo->full(mvm_id_fifo_full);
  mvm_id_fifo->almost_full(mvm_id_fifo_almost_full);
  mvm_id_fifo->empty(mvm_id_fifo_empty);
  mvm_id_fifo->odata(mvm_id_fifo_odata);

  SC_METHOD(Assign);
  sensitive << rst << inst_fifo_empty << inst_fifo_odata << inst_fifo_almost_full
            << layer_id_fifo_empty << layer_id_fifo_odata << layer_id_fifo_almost_full
            << mvm_id_fifo_empty << mvm_id_fifo_odata << mvm_id_fifo_almost_full
            << tx_interface.tvalid << tx_interface.tready;

  this->RegisterModuleInfo();
}

inst_loader::~inst_loader() { 
  delete inst_fifo;
  delete layer_id_fifo;
  delete mvm_id_fifo; 
}

void inst_loader::Assign() {
  if (rst.read()) {
    tx_interface.tvalid.write(false);
    inst_fifo_rdy.write(false);
    layer_id_fifo_rdy.write(false);
    mvm_id_fifo_rdy.write(false);
  } else {
    mvm_inst tx_tdata = inst_fifo_odata.read();
    sc_bv<AXIS_MAX_DATAW> tx_tdata_bv;
    if (!inst_fifo_empty.read()) {
      tx_tdata_bv = tx_tdata.to_bv();
      tx_interface.tdata.write(tx_tdata_bv);
      tx_interface.tuser.write(0);
      tx_interface.tid.write(0);
      std::string dest_name =
          "layer" + std::to_string(layer_id_fifo_odata.read()) + 
          "_mvm" + std::to_string(mvm_id_fifo_odata.read()) + ".rx_interface";
      tx_interface.tdest.write(radsim_design.GetPortDestinationID(dest_name));
    }
    tx_interface.tvalid.write(!inst_fifo_empty.read());

    inst_fifo_pop.write(tx_interface.tvalid.read() && tx_interface.tready.read());
    layer_id_fifo_pop.write(tx_interface.tvalid.read() && tx_interface.tready.read());
    mvm_id_fifo_pop.write(tx_interface.tvalid.read() && tx_interface.tready.read());

    inst_fifo_rdy.write(!inst_fifo_almost_full);
    layer_id_fifo_rdy.write(!layer_id_fifo_almost_full);
    mvm_id_fifo_rdy.write(!mvm_id_fifo_almost_full);
  }
}

void inst_loader::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;

  port_name = module_name + ".inst_dispatch";
  RegisterAxisMasterPort(port_name, &tx_interface, 512, 0);
}