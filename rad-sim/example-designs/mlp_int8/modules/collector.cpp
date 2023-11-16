#include "collector.hpp"

collector::collector(const sc_module_name &name)
    : RADSimModule(name), rst("rst"), data_fifo_rdy("data_fifo_rdy"),
      data_fifo_ren("data_fifo_ren"), data_fifo_rdata("data_fifo_rdata") {

  module_name = name;

  char fifo_name[25];
  std::string fifo_name_str;
  fifo_name_str = "collector_data_fifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  data_fifo =
      new fifo<data_vector<sc_int<IPRECISION>>>(fifo_name, FIFOD, FIFOD - 1);
  data_fifo->clk(clk);
  data_fifo->rst(rst);
  data_fifo->push(data_fifo_wen_signal);
  data_fifo->pop(data_fifo_ren);
  data_fifo->idata(data_fifo_wdata_signal);
  data_fifo->full(data_fifo_full_signal);
  data_fifo->almost_full(data_fifo_almost_full_signal);
  data_fifo->empty(data_fifo_empty_signal);
  data_fifo->odata(data_fifo_rdata);

  SC_METHOD(Assign);
  sensitive << rst << data_fifo_empty_signal << data_fifo_almost_full_signal
            << rx_interface.tvalid << rx_interface.tdata << rx_interface.tready;

  this->RegisterModuleInfo();
}

collector::~collector() { delete data_fifo; }

void collector::Assign() {
  if (rst.read()) {
    rx_interface.tready.write(false);
    data_fifo_rdy.write(false);
  } else {
    rx_interface.tready.write(!data_fifo_almost_full_signal);
    data_fifo_wen_signal.write(rx_interface.tvalid.read() &&
                               rx_interface.tready.read());
    data_fifo_rdy.write(!data_fifo_empty_signal);

    data_vector<sc_int<IPRECISION>> tx_tdata(LANES);
    sc_bv<AXIS_MAX_DATAW> tx_tdata_bv = rx_interface.tdata.read();
    if (rx_interface.tvalid.read() && rx_interface.tready.read()) {
      for (unsigned int lane_id = 0; lane_id < LANES; lane_id++) {
        tx_tdata[lane_id] =
            tx_tdata_bv.range((lane_id + 1) * IPRECISION - 1, lane_id * IPRECISION).to_int();
      }
      data_fifo_wdata_signal.write(tx_tdata);
    }
  }
}

void collector::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;

  port_name = module_name + ".data_collect";
  RegisterAxisSlavePort(port_name, &rx_interface, 512, 0);
}