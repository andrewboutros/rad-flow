#include "dispatcher.hpp"

dispatcher::dispatcher(const sc_module_name &name, unsigned int id)
    : RADSimModule(name), rst("rst"), data_fifo_rdy("data_fifo_rdy"),
      data_fifo_wen("data_fifo_wen"), data_fifo_wdata("data_fifo_wdata") {

  module_name = name;
  dispatcher_id = id;

  char fifo_name[25];
  std::string fifo_name_str;
  fifo_name_str = "dispatcher" + std::to_string(dispatcher_id) + "_data_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  data_fifo =
      new fifo<sc_int<32>>(fifo_name, FIFO_SIZE, LANES, FIFO_SIZE - 1, 0);
  data_fifo->clk(clk);
  data_fifo->rst(rst);
  data_fifo->wen(data_fifo_wen);
  data_fifo->ren(data_fifo_ren_signal);
  data_fifo->wdata(data_fifo_wdata);
  data_fifo->full(data_fifo_full_signal);
  data_fifo->almost_full(data_fifo_almost_full_signal);
  data_fifo->empty(data_fifo_empty_signal);
  data_fifo->almost_empty(data_fifo_almost_empty_signal);
  data_fifo->rdata(data_fifo_rdata_signal);

  SC_METHOD(Assign);
  sensitive << rst << data_fifo_empty_signal << data_fifo_rdata_signal
            << data_fifo_almost_full_signal << tx_interface.tvalid
            << tx_interface.tready;

  this->RegisterModuleInfo();
}

dispatcher::~dispatcher() { delete data_fifo; }

void dispatcher::Assign() {
  if (rst.read()) {
    tx_interface.tvalid.write(false);
    data_fifo_rdy.write(false);
  } else {
    data_vector<sc_int<32>> tx_tdata = data_fifo_rdata_signal.read();
    sc_bv<AXIS_MAX_DATAW> tx_tdata_bv;
    if (tx_tdata.size() > 0) {
      for (unsigned int lane_id = 0; lane_id < LANES; lane_id++) {
        tx_tdata_bv.range((lane_id + 1) * 32 - 1, lane_id * 32) =
            tx_tdata[lane_id];
      }
      tx_interface.tvalid.write(!data_fifo_empty_signal);
      tx_interface.tdata.write(tx_tdata_bv);
      tx_interface.tuser.write(3 << 13);
      tx_interface.tid.write(0);
      std::string dest_name =
          "layer0_mvm" + std::to_string(dispatcher_id) + ".rx_interface";
      tx_interface.tdest.write(radsim_design.GetPortDestinationID(dest_name));
      // std::cout << "Dispatcher " << dispatcher_id << " pushed data into the
      // NoC with dest "
      //   << radsim_design.GetPortDestinationID(dest_name) << "!" << std::endl;
    } else {
      tx_interface.tvalid.write(false);
    }

    data_fifo_ren_signal.write(tx_interface.tvalid.read() &&
                               tx_interface.tready.read());
    data_fifo_rdy.write(!data_fifo_almost_full_signal);
  }
}

void dispatcher::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;

  port_name = module_name + ".data_dispatch";
  RegisterAxisMasterPort(port_name, &tx_interface, 512, 0);
}