#pragma once

#include <systemc.h>
#include <iostream>
#include <design_context.hpp>
#include <vector>
#include <string>
#include <radsim_cluster.hpp>
#include <radsim_config.hpp>
#include <axis_interface.hpp>
#include <radsim_utils.hpp>
#include <portal.hpp>
#include <radsim_defines.hpp>

struct axis_fields {
  bool tvalid;
  bool tready;
  sc_bv<AXIS_MAX_DATAW> tdata;
  sc_bv<AXIS_STRBW> tstrb;
  sc_bv<AXIS_KEEPW> tkeep;
  bool tlast;
  sc_bv<AXIS_IDW> tid;
  sc_bv<AXIS_DESTW> tdest;
  sc_bv<AXIS_USERW> tuser;
  //needed to create sc_fifo of custom struct type
  friend std::ostream& operator<<(std::ostream& os, const axis_fields& I);
};


//Network for communication between RADs
class RADSimInterRad : public sc_module {
    private:
        RADSimCluster* cluster;
        sc_vector<sc_signal<sc_bv<AXIS_MAX_DATAW>>> all_signals{"all_signals"};
        int bw_limit = 0;
    public:
        int num_rads;
        sc_in<bool> clk;
        std::vector<sc_fifo<axis_fields>*> fifos;

        //for axi interfaces
        std::vector<axis_signal*> all_axis_master_signals;
        std::vector<axis_signal*> all_axis_slave_signals;
        std::vector<axis_slave_port*> all_axis_slave_ports;
        std::vector<axis_master_port*> all_axis_master_ports;

        //for rising edge detection on each interface
        std::vector<bool> prev_valid;

        //using vector of vectors bc dynamic sizing based on num_rads, allows pushback and erase, and faster incrementing (we increment more often than erase from front) than std::deque
        std::vector<std::vector<int>> fifos_latency_counters;

        RADSimInterRad(const sc_module_name &name, sc_clock *inter_rad_clk, RADSimCluster* cluster);
        ~RADSimInterRad();
        void ConnectRadAxi(int i);
        void writeFifo();
        void readFifo();
        SC_HAS_PROCESS(RADSimInterRad);
};
