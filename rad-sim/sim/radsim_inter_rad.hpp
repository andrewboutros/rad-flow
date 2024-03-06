#pragma once

#include <systemc.h>
#include <iostream>
#include <design_context.hpp>
#include <vector>
#include <string>
#include <radsim_cluster.hpp>
#include <radsim_config.hpp>
#include <axis_interface.hpp>

#define DATAW 128
#define NUM_SLOTS 2 //number of fifo slots

struct axis_fields {
  bool tvalid;
  bool tready;
  sc_bv<DATAW> tdata;
  sc_bv<AXIS_STRBW> tstrb;
  sc_bv<AXIS_KEEPW> tkeep;
  bool tlast;
  sc_bv<AXIS_IDW> tid;
  sc_bv<AXIS_DESTW> tdest;
  sc_bv<AXIS_USERW> tuser;
  //needed to create sc_fifo of custom struct type
  friend std::ostream& operator<<(std::ostream& os, const axis_fields& I);
};

class RADSimInterRad : public sc_module {
    private:
        RADSimCluster* cluster;
        //std::vector<sc_signal<bool>> all_signals;
        sc_vector<sc_signal<sc_bv<DATAW>>> all_signals{"all_signals"};
        //sc_fifo<sc_bv<DATAW>> data_in_rad1 = sc_fifo<sc_bv<DATAW>>(2); //2 slots for now
        //sc_vector<sc_fifo<sc_bv<DATAW>>> switch_port_fifos{"switch_port_fifos"};
    public:
        sc_in<bool> clk;
        //std::vector<sc_fifo<sc_bv<DATAW>>*> fifos; //works but replacing with struct elems
        std::vector<sc_fifo<axis_fields>*> fifos;

        //for axi interfaces
        std::vector<axis_signal*> all_axis_master_signals;
        std::vector<axis_signal*> all_axis_slave_signals;
        std::vector<axis_slave_port*> all_axis_slave_ports;
        std::vector<axis_master_port*> all_axis_master_ports;

        RADSimInterRad(const sc_module_name &name, sc_clock *inter_rad_clk, RADSimCluster* cluster);
        ~RADSimInterRad();
        void ConnectRadPair(int i, int j);
        void ConnectRadAxi(int i);
        /*sc_signal<bool> in_i_out_j;
	    sc_signal<bool> in_j_out_i; */
        void writeFifo();
        void readFifo();
        SC_HAS_PROCESS(RADSimInterRad);
};