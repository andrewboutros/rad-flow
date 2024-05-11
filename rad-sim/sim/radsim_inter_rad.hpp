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

#define DATAW 16*32 //changed to match dlrm defines file //128
#define NUM_SLOTS 1000 //5 //number of fifo slots, for now = NUM_ADDENDS
#define DEST_RAD_LSB 0
#define DEST_RAD_MSB 7

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
        //for latency
        float latency_sec = 2.1 * pow(10, -6); //5.0*1 * pow(10, -9); //2.6 * pow(10, -6); //do not currently support zero latency -- I could implement by bypassing FIFO
        float period_sec = 5.0 * pow(10, -9);
        int target_delay = ceil(latency_sec/period_sec); //number of cycles to delay
        int bw_limit = 0;
    public:
        int num_rads;
        sc_in<bool> clk;
        //std::vector<sc_fifo<sc_bv<DATAW>>*> fifos; //works but replacing with struct elems
        std::vector<sc_fifo<axis_fields>*> fifos;

        //for axi interfaces
        std::vector<axis_signal*> all_axis_master_signals;
        std::vector<axis_signal*> all_axis_slave_signals;
        std::vector<axis_slave_port*> all_axis_slave_ports;
        std::vector<axis_master_port*> all_axis_master_ports;

        //for rising edge detection on each interface
        std::vector<bool> prev_valid;

        //for latency counter
        //std::vector<std::array<int, NUM_SLOTS>> fifos_latency_counters;
        //using vector of vectors bc dynamic sizing based on num_rads, allows pushback and erase, and faster incrementing (we increment more often than erase from front) than std::deque
        std::vector<std::vector<int>> fifos_latency_counters;

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