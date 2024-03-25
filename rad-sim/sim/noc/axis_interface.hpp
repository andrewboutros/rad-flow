#pragma once

#include <radsim_defines.hpp>
#include <radsim_utils.hpp>
#include <systemc.h>

// Encapsulation of all AXI-stream master interface ports in a single struct
struct axis_master_port {
  sc_out<bool> tvalid;
  sc_in<bool> tready;
  sc_out<sc_bv<AXIS_MAX_DATAW>> tdata;
  sc_out<sc_bv<AXIS_STRBW>> tstrb;
  sc_out<sc_bv<AXIS_KEEPW>> tkeep;
  sc_out<bool> tlast;
  sc_out<sc_bv<AXIS_IDW>> tid;
  sc_out<sc_bv<AXIS_DESTW+RAD_DESTW>> tdest;
  sc_out<sc_bv<AXIS_USERW>> tuser;

  axis_master_port()
      : tvalid(), tready(), tdata(), tstrb(), tkeep(), tlast(), tid(), tdest(),
        tuser() {}
  axis_master_port(const sc_module_name &name)
      : tvalid(), tready(), tdata(), tstrb(), tkeep(), tlast(), tid(), tdest(),
        tuser() {
    (void)name;
  }

  // Helper function for reseting all output ports of an AXI-stream master
  // interface
  void Reset() {
    tvalid.write(false);
    tdata.write(0);
    tstrb.write(0);
    tkeep.write(0);
    tlast.write(false);
    tid.write(0);
    tdest.write(0);
    tuser.write(0);
  }

  // Helper function for connecting two AXI-stream master ports in module
  // hierarchy
  void ConnectToPort(axis_master_port &p) {
    this->tvalid(p.tvalid);
    this->tready(p.tready);
    this->tdata(p.tdata);
    this->tstrb(p.tstrb);
    this->tkeep(p.tkeep);
    this->tlast(p.tlast);
    this->tid(p.tid);
    this->tdest(p.tdest);
    this->tuser(p.tuser);
  }
};

// Encapsulation of all AXI-stream slave interface ports in a single struct
struct axis_slave_port {
  sc_in<bool> tvalid;
  sc_out<bool> tready;
  sc_in<sc_bv<AXIS_MAX_DATAW>> tdata;
  sc_in<sc_bv<AXIS_STRBW>> tstrb;
  sc_in<sc_bv<AXIS_KEEPW>> tkeep;
  sc_in<bool> tlast;
  sc_in<sc_bv<AXIS_IDW>> tid;
  sc_in<sc_bv<AXIS_DESTW+RAD_DESTW>> tdest;
  sc_in<sc_bv<AXIS_USERW>> tuser;

  axis_slave_port()
      : tvalid(), tready(), tdata(), tstrb(), tkeep(), tlast(), tid(), tdest(),
        tuser() {}
  axis_slave_port(const sc_module_name &name)
      : tvalid(), tready(), tdata(), tstrb(), tkeep(), tlast(), tid(), tdest(),
        tuser() {
    (void)name;
  }

  // Helper function for reseting all output ports of an AXI-stream slave
  // interface
  void Reset() { tready.write(true); }

  // Helper function for connecting two AXI-stream slave ports in module
  // hierarchy
  void ConnectToPort(axis_slave_port &p) {
    this->tvalid(p.tvalid);
    this->tready(p.tready);
    this->tdata(p.tdata);
    this->tstrb(p.tstrb);
    this->tkeep(p.tkeep);
    this->tlast(p.tlast);
    this->tid(p.tid);
    this->tdest(p.tdest);
    this->tuser(p.tuser);
  }
};

// Encapsulation of all AXI-stream signals in a single struct
struct axis_signal {
  sc_signal<bool> tvalid;
  sc_signal<bool> tready;
  sc_signal<sc_bv<AXIS_MAX_DATAW>> tdata;
  sc_signal<sc_bv<AXIS_STRBW>> tstrb;
  sc_signal<sc_bv<AXIS_KEEPW>> tkeep;
  sc_signal<bool> tlast;
  sc_signal<sc_bv<AXIS_IDW>> tid;
  sc_signal<sc_bv<AXIS_DESTW+RAD_DESTW>> tdest;
  sc_signal<sc_bv<AXIS_USERW>> tuser;

  axis_signal()
      : tvalid(), tready(), tdata(), tstrb(), tkeep(), tlast(), tid(), tdest(),
        tuser() {}
  axis_signal(const axis_signal &sig) {
    tvalid = sig.tvalid;
    tready = sig.tready;
    tdata = sig.tdata;
    tstrb = sig.tstrb;
    tkeep = sig.tkeep;
    tlast = sig.tlast;
    tid = sig.tid;
    tdest = sig.tdest;
    tuser = sig.tuser;
  }
  axis_signal(const sc_module_name &name)
      : tvalid(), tready(), tdata(), tstrb(), tkeep(), tlast(), tid(), tdest(),
        tuser() {
    (void)name;
  }

  // Helper function for connecting the AXI-stream master and slave ports of two
  // modules
  void Connect(axis_master_port &m, axis_slave_port &s) {
    //AKB ADDED few lines below TO TEST
    //std::cout << "Here in Connect in axis_interface.hpp" << endl;
    //std::cout << "master " << &m << " and slave " << &s << endl;
    // Connect signal to master port
    m.tvalid(tvalid);
    m.tready(tready);
    m.tdata(tdata);
    m.tstrb(tstrb);
    m.tkeep(tkeep);
    m.tlast(tlast);
    m.tid(tid);
    m.tdest(tdest);
    m.tuser(tuser);
    // Connect signal to slave port
    s.tvalid(tvalid);
    s.tready(tready);
    s.tdata(tdata);
    s.tstrb(tstrb);
    s.tkeep(tkeep);
    s.tlast(tlast);
    s.tid(tid);
    s.tdest(tdest);
    s.tuser(tuser);
  }

  void Reset() {
    tvalid.write(false);
    tready.write(false);
    tdata.write(0);
    tstrb.write(0);
    tkeep.write(0);
    tlast.write(false);
    tid.write(0);
    tdest.write(0);
    tuser.write(0);
  }
};