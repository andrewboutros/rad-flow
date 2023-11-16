#include "weight_loader.hpp"

weight_loader::weight_loader(const sc_module_name &name)
    : RADSimModule(name), 
      rst("rst"), 
      weight_fifo_rdy("weight_fifo_rdy"),
      weight_fifo_wen("weight_fifo_wen"), 
      weight_fifo_wdata("weight_fifo_wdata"),
      rf_id_fifo_rdy("rf_id_fifo_rdy"),
      rf_id_fifo_wen("rf_id_fifo_wen"), 
      rf_id_fifo_wdata("rf_id_fifo_wdata"),
      rf_addr_fifo_rdy("rf_addr_fifo_rdy"),
      rf_addr_fifo_wen("rf_addr_fifo_wen"),
      rf_addr_fifo_wdata("rf_addr_fifo_wdata"),
      layer_id_fifo_rdy("layer_id_fifo_rdy"),
      layer_id_fifo_wen("layer_id_fifo_wen"),
      layer_id_fifo_wdata("layer_id_fifo_wdata"),
      mvm_id_fifo_rdy("mvm_id_fifo_rdy"),
      mvm_id_fifo_wen("mvm_id_fifo_wen"),
      mvm_id_fifo_wdata("mvm_id_fifo_wdata") {

  module_name = name;

  char fifo_name[25];
  std::string fifo_name_str;

  fifo_name_str = "weight_loader_weight_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  weight_fifo = new fifo<data_vector<sc_int<IPRECISION>>>(fifo_name, FIFOD, FIFOD - 1);
  weight_fifo->clk(clk);
  weight_fifo->rst(rst);
  weight_fifo->push(weight_fifo_wen);
  weight_fifo->pop(weight_fifo_pop);
  weight_fifo->idata(weight_fifo_wdata);
  weight_fifo->full(weight_fifo_full);
  weight_fifo->almost_full(weight_fifo_almost_full);
  weight_fifo->empty(weight_fifo_empty);
  weight_fifo->odata(weight_fifo_odata);

  fifo_name_str = "weight_loader_rf_id_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  rf_id_fifo = new fifo<uint8_t>(fifo_name, FIFOD, FIFOD - 1);
  rf_id_fifo->clk(clk);
  rf_id_fifo->rst(rst);
  rf_id_fifo->push(rf_id_fifo_wen);
  rf_id_fifo->pop(rf_id_fifo_pop);
  rf_id_fifo->idata(rf_id_fifo_wdata);
  rf_id_fifo->full(rf_id_fifo_full);
  rf_id_fifo->almost_full(rf_id_fifo_almost_full);
  rf_id_fifo->empty(rf_id_fifo_empty);
  rf_id_fifo->odata(rf_id_fifo_odata);

  fifo_name_str = "weight_loader_rf_addr_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  rf_addr_fifo = new fifo<sc_uint<RF_ADDRW>>(fifo_name, FIFOD, FIFOD - 1);
  rf_addr_fifo->clk(clk);
  rf_addr_fifo->rst(rst);
  rf_addr_fifo->push(rf_addr_fifo_wen);
  rf_addr_fifo->pop(rf_addr_fifo_pop);
  rf_addr_fifo->idata(rf_addr_fifo_wdata);
  rf_addr_fifo->full(rf_addr_fifo_full);
  rf_addr_fifo->almost_full(rf_addr_fifo_almost_full);
  rf_addr_fifo->empty(rf_addr_fifo_empty);
  rf_addr_fifo->odata(rf_addr_fifo_odata);

  fifo_name_str = "weight_loader_layer_id_fifo";
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

  fifo_name_str = "weight_loader_mvm_id_fifo";
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
  sensitive << rst << weight_fifo_empty << weight_fifo_odata << weight_fifo_almost_full
            << rf_id_fifo_empty << rf_id_fifo_odata << rf_id_fifo_almost_full 
            << rf_addr_fifo_empty << rf_addr_fifo_odata << rf_addr_fifo_almost_full 
            << layer_id_fifo_empty << layer_id_fifo_odata << layer_id_fifo_almost_full
            << mvm_id_fifo_empty << mvm_id_fifo_odata << mvm_id_fifo_almost_full
            << tx_interface.tvalid << tx_interface.tready;

  this->RegisterModuleInfo();
}

weight_loader::~weight_loader() { 
  delete weight_fifo;
  delete rf_id_fifo;
  delete rf_addr_fifo;
  delete layer_id_fifo;
  delete mvm_id_fifo; 
}

void weight_loader::Assign() {
  if (rst.read()) {
    tx_interface.tvalid.write(false);
    weight_fifo_rdy.write(false);
    rf_id_fifo_rdy.write(false);
    rf_addr_fifo_rdy.write(false);
    layer_id_fifo_rdy.write(false);
    mvm_id_fifo_rdy.write(false);
  } else {
    data_vector<sc_int<IPRECISION>> tx_tdata = weight_fifo_odata.read();
    sc_bv<AXIS_MAX_DATAW> tx_tdata_bv;
    if (tx_tdata.size() > 0) {
      for (unsigned int lane_id = 0; lane_id < LANES; lane_id++) {
        tx_tdata_bv.range((lane_id + 1) * IPRECISION - 1, lane_id * IPRECISION) =
            tx_tdata[lane_id];
      }
      tx_interface.tvalid.write(!weight_fifo_empty);
      tx_interface.tdata.write(tx_tdata_bv);
      sc_bv<AXIS_USERW> tuser_value;
      tuser_value.range(8, 0) = rf_addr_fifo_odata.read().to_uint();
      tuser_value.range(10, 9) = 3;
      for(unsigned int i = 0; i < DPES; i++) tuser_value.range(11+i, 11+i) = 0;
      tuser_value.range(11+rf_id_fifo_odata.read(), 11+rf_id_fifo_odata.read()) = 1;
      tx_interface.tid.write(0);
      tx_interface.tuser.write(tuser_value);
      std::string dest_name =
          "layer" + std::to_string(layer_id_fifo_odata.read()) + 
          "_mvm" + std::to_string(mvm_id_fifo_odata.read()) + ".rx_interface";
      tx_interface.tdest.write(radsim_design.GetPortDestinationID(dest_name));
    } else {
      tx_interface.tvalid.write(false);
    }

    weight_fifo_pop.write(tx_interface.tvalid.read() && tx_interface.tready.read());
    rf_id_fifo_pop.write(tx_interface.tvalid.read() && tx_interface.tready.read());
    rf_addr_fifo_pop.write(tx_interface.tvalid.read() && tx_interface.tready.read());
    layer_id_fifo_pop.write(tx_interface.tvalid.read() && tx_interface.tready.read());
    mvm_id_fifo_pop.write(tx_interface.tvalid.read() && tx_interface.tready.read());

    weight_fifo_rdy.write(!weight_fifo_almost_full);
    rf_id_fifo_rdy.write(!rf_id_fifo_almost_full);
    rf_addr_fifo_rdy.write(!rf_addr_fifo_almost_full);
    layer_id_fifo_rdy.write(!layer_id_fifo_almost_full);
    mvm_id_fifo_rdy.write(!mvm_id_fifo_almost_full);
  }
}

void weight_loader::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;

  port_name = module_name + ".weight_dispatch";
  RegisterAxisMasterPort(port_name, &tx_interface, 512, 0);
}