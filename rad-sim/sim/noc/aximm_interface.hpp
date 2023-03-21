#pragma once

#include <radsim_defines.hpp>
#include <radsim_utils.hpp>
#include <systemc.h>

// Encapsulation of all AXI-MM master interface ports in a single struct
// An AXI-MM master can send read/write requests and receive responses
struct aximm_master_port {
  // Write address channel
  sc_out<bool> awvalid;
  sc_in<bool> awready;
  sc_out<sc_bv<AXI4_IDW>> awid;
  sc_out<sc_bv<AXI4_ADDRW>> awaddr;
  sc_out<sc_bv<AXI4_LENW>> awlen;
  sc_out<sc_bv<AXI4_SIZEW>> awsize;
  sc_out<sc_bv<AXI4_BURSTW>> awburst;
  sc_out<sc_bv<AXI4_USERW>> awuser;
  // Write data channel
  sc_out<bool> wvalid;
  sc_in<bool> wready;
  sc_out<sc_bv<AXI4_IDW>> wid;
  sc_out<sc_bv<AXI4_MAX_DATAW>> wdata;
  sc_out<bool> wlast;
  sc_out<sc_bv<AXI4_USERW>> wuser;
  // Write response channel
  sc_in<bool> bvalid;
  sc_out<bool> bready;
  sc_in<sc_bv<AXI4_IDW>> bid;
  sc_in<sc_bv<AXI4_RESPW>> bresp;
  sc_in<sc_bv<AXI4_USERW>> buser;
  // Read address channel
  sc_out<bool> arvalid;
  sc_in<bool> arready;
  sc_out<sc_bv<AXI4_IDW>> arid;
  sc_out<sc_bv<AXI4_ADDRW>> araddr;
  sc_out<sc_bv<AXI4_LENW>> arlen;
  sc_out<sc_bv<AXI4_SIZEW>> arsize;
  sc_out<sc_bv<AXI4_BURSTW>> arburst;
  sc_out<sc_bv<AXI4_USERW>> aruser;
  // Read data channel
  sc_in<bool> rvalid;
  sc_out<bool> rready;
  sc_in<sc_bv<AXI4_IDW>> rid;
  sc_in<sc_bv<AXI4_MAX_DATAW>> rdata;
  sc_in<sc_bv<AXI4_RESPW>> rresp;
  sc_in<bool> rlast;
  sc_in<sc_bv<AXI4_USERW>> ruser;

  aximm_master_port()
      : awvalid(), awready(), awid(), awaddr(), awlen(), awsize(), awburst(),
        awuser(), wvalid(), wready(), wid(), wdata(), wlast(), wuser(),
        bvalid(), bready(), bid(), bresp(), buser(), arvalid(), arready(),
        arid(), araddr(), arlen(), arsize(), arburst(), aruser(), rvalid(),
        rready(), rid(), rdata(), rresp(), rlast(), ruser() {}
  aximm_master_port(const sc_module_name &name)
      : awvalid(), awready(), awid(), awaddr(), awlen(), awsize(), awburst(),
        awuser(), wvalid(), wready(), wid(), wdata(), wlast(), wuser(),
        bvalid(), bready(), bid(), bresp(), buser(), arvalid(), arready(),
        arid(), araddr(), arlen(), arsize(), arburst(), aruser(), rvalid(),
        rready(), rid(), rdata(), rresp(), rlast(), ruser() {
    (void)name;
  }

  // Helper function for reseting all output ports of an AXI-MM master interface
  void Reset() {
    awvalid.write(false);
    awid.write(0);
    awaddr.write(0);
    awlen.write(0);
    awsize.write(0);
    awburst.write(0);
    awuser.write(0);
    wvalid.write(false);
    wid.write(0);
    wdata.write(0);
    wlast.write(false);
    wuser.write(0);
    bready.write(false);
    arvalid.write(false);
    arid.write(0);
    araddr.write(0);
    arlen.write(0);
    arsize.write(0);
    arburst.write(0);
    aruser.write(0);
    rready.write(false);
  }

  // Helper function for connecting two AXI-MM master ports in module hierarchy
  void ConnectToPort(aximm_master_port &p) {
    // Write address channel
    this->awvalid(p.awvalid);
    this->awready(p.awready);
    this->awid(p.awid);
    this->awaddr(p.awaddr);
    this->awlen(p.awlen);
    this->awsize(p.awsize);
    this->awburst(p.awburst);
    this->awuser(p.awuser);
    // Write data channel
    this->wvalid(p.wvalid);
    this->wready(p.wready);
    this->wid(p.wid);
    this->wdata(p.wdata);
    this->wlast(p.wlast);
    this->wuser(p.wuser);
    // Write response channel
    this->bvalid(p.bvalid);
    this->bready(p.bready);
    this->bid(p.bid);
    this->bresp(p.bresp);
    this->buser(p.buser);
    // Read address channel
    this->arvalid(p.arvalid);
    this->arready(p.arready);
    this->arid(p.arid);
    this->araddr(p.araddr);
    this->arlen(p.arlen);
    this->arsize(p.arsize);
    this->arburst(p.arburst);
    this->aruser(p.aruser);
    // Read data channel
    this->rvalid(p.rvalid);
    this->rready(p.rready);
    this->rid(p.rid);
    this->rdata(p.rdata);
    this->rresp(p.rresp);
    this->rlast(p.rlast);
    this->ruser(p.ruser);
  }
};

// Encapsulation of all AXI-MM slave interface ports in a single struct
// An AXI-MM slave can receive read/write requests and send responses
struct aximm_slave_port {
  // Write address channel
  sc_in<bool> awvalid;
  sc_out<bool> awready;
  sc_in<sc_bv<AXI4_IDW>> awid;
  sc_in<sc_bv<AXI4_ADDRW>> awaddr;
  sc_in<sc_bv<AXI4_LENW>> awlen;
  sc_in<sc_bv<AXI4_SIZEW>> awsize;
  sc_in<sc_bv<AXI4_BURSTW>> awburst;
  sc_in<sc_bv<AXI4_USERW>> awuser;
  // Write data channel
  sc_in<bool> wvalid;
  sc_out<bool> wready;
  sc_in<sc_bv<AXI4_IDW>> wid;
  sc_in<sc_bv<AXI4_MAX_DATAW>> wdata;
  sc_in<bool> wlast;
  sc_in<sc_bv<AXI4_USERW>> wuser;
  // Write response channel
  sc_out<bool> bvalid;
  sc_in<bool> bready;
  sc_out<sc_bv<AXI4_IDW>> bid;
  sc_out<sc_bv<AXI4_RESPW>> bresp;
  sc_out<sc_bv<AXI4_USERW>> buser;
  // Read address channel
  sc_in<bool> arvalid;
  sc_out<bool> arready;
  sc_in<sc_bv<AXI4_IDW>> arid;
  sc_in<sc_bv<AXI4_ADDRW>> araddr;
  sc_in<sc_bv<AXI4_LENW>> arlen;
  sc_in<sc_bv<AXI4_SIZEW>> arsize;
  sc_in<sc_bv<AXI4_BURSTW>> arburst;
  sc_in<sc_bv<AXI4_USERW>> aruser;
  // Read data channel
  sc_out<bool> rvalid;
  sc_in<bool> rready;
  sc_out<sc_bv<AXI4_IDW>> rid;
  sc_out<sc_bv<AXI4_MAX_DATAW>> rdata;
  sc_out<sc_bv<AXI4_RESPW>> rresp;
  sc_out<bool> rlast;
  sc_out<sc_bv<AXI4_USERW>> ruser;

  aximm_slave_port()
      : awvalid(), awready(), awid(), awaddr(), awlen(), awsize(), awburst(),
        awuser(), wvalid(), wready(), wid(), wdata(), wlast(), wuser(),
        bvalid(), bready(), bid(), bresp(), buser(), arvalid(), arready(),
        arid(), araddr(), arlen(), arsize(), arburst(), aruser(), rvalid(),
        rready(), rid(), rdata(), rresp(), rlast(), ruser() {}
  aximm_slave_port(const sc_module_name &name)
      : awvalid(), awready(), awid(), awaddr(), awlen(), awsize(), awburst(),
        awuser(), wvalid(), wready(), wid(), wdata(), wlast(), wuser(),
        bvalid(), bready(), bid(), bresp(), buser(), arvalid(), arready(),
        arid(), araddr(), arlen(), arsize(), arburst(), aruser(), rvalid(),
        rready(), rid(), rdata(), rresp(), rlast(), ruser() {
    (void)name;
  }

  // Helper function for reseting all output ports of an AXI-MM slave interface
  void Reset() {
    awready.write(false);
    wready.write(false);
    bvalid.write(false);
    bid.write(0);
    bresp.write(0);
    buser.write(0);
    arready.write(false);
    rvalid.write(false);
    rid.write(0);
    rdata.write(0);
    rresp.write(0);
    rlast.write(false);
    ruser.write(0);
  }

  // Helper function for connecting two AXI-MM slave ports in module hierarchy
  void ConnectToPort(aximm_slave_port &p) {
    // Write address channel
    this->awvalid(p.awvalid);
    this->awready(p.awready);
    this->awid(p.awid);
    this->awaddr(p.awaddr);
    this->awlen(p.awlen);
    this->awsize(p.awsize);
    this->awburst(p.awburst);
    this->awuser(p.awuser);
    // Write data channel
    this->wvalid(p.wvalid);
    this->wready(p.wready);
    this->wid(p.wid);
    this->wdata(p.wdata);
    this->wlast(p.wlast);
    this->wuser(p.wuser);
    // Write response channel
    this->bvalid(p.bvalid);
    this->bready(p.bready);
    this->bid(p.bid);
    this->bresp(p.bresp);
    this->buser(p.buser);
    // Read address channel
    this->arvalid(p.arvalid);
    this->arready(p.arready);
    this->arid(p.arid);
    this->araddr(p.araddr);
    this->arlen(p.arlen);
    this->arsize(p.arsize);
    this->arburst(p.arburst);
    this->aruser(p.aruser);
    // Read data channel
    this->rvalid(p.rvalid);
    this->rready(p.rready);
    this->rid(p.rid);
    this->rdata(p.rdata);
    this->rresp(p.rresp);
    this->rlast(p.rlast);
    this->ruser(p.ruser);
  }
};

// Encapsulation of all AXI-MM signals in a single struct
struct aximm_signal {
  // Write address channel
  sc_signal<bool> awvalid;
  sc_signal<bool> awready;
  sc_signal<sc_bv<AXI4_IDW>> awid;
  sc_signal<sc_bv<AXI4_ADDRW>> awaddr;
  sc_signal<sc_bv<AXI4_LENW>> awlen;
  sc_signal<sc_bv<AXI4_SIZEW>> awsize;
  sc_signal<sc_bv<AXI4_BURSTW>> awburst;
  sc_signal<sc_bv<AXI4_USERW>> awuser;
  // Write data channel
  sc_signal<bool> wvalid;
  sc_signal<bool> wready;
  sc_signal<sc_bv<AXI4_IDW>> wid;
  sc_signal<sc_bv<AXI4_MAX_DATAW>> wdata;
  sc_signal<bool> wlast;
  sc_signal<sc_bv<AXI4_USERW>> wuser;
  // Write response channel
  sc_signal<bool> bvalid;
  sc_signal<bool> bready;
  sc_signal<sc_bv<AXI4_IDW>> bid;
  sc_signal<sc_bv<AXI4_RESPW>> bresp;
  sc_signal<sc_bv<AXI4_USERW>> buser;
  // Read address channel
  sc_signal<bool> arvalid;
  sc_signal<bool> arready;
  sc_signal<sc_bv<AXI4_IDW>> arid;
  sc_signal<sc_bv<AXI4_ADDRW>> araddr;
  sc_signal<sc_bv<AXI4_LENW>> arlen;
  sc_signal<sc_bv<AXI4_SIZEW>> arsize;
  sc_signal<sc_bv<AXI4_BURSTW>> arburst;
  sc_signal<sc_bv<AXI4_USERW>> aruser;
  // Read data channel
  sc_signal<bool> rvalid;
  sc_signal<bool> rready;
  sc_signal<sc_bv<AXI4_IDW>> rid;
  sc_signal<sc_bv<AXI4_MAX_DATAW>> rdata;
  sc_signal<sc_bv<AXI4_RESPW>> rresp;
  sc_signal<bool> rlast;
  sc_signal<sc_bv<AXI4_USERW>> ruser;

  aximm_signal()
      : awvalid(), awready(), awid(), awaddr(), awlen(), awsize(), awburst(),
        awuser(), wvalid(), wready(), wid(), wdata(), wlast(), wuser(),
        bvalid(), bready(), bid(), bresp(), buser(), arvalid(), arready(),
        arid(), araddr(), arlen(), arsize(), arburst(), aruser(), rvalid(),
        rready(), rid(), rdata(), rresp(), rlast(), ruser() {}
  aximm_signal(const aximm_signal &sig) {
    // Write address channel
    awvalid = sig.awvalid;
    awready = sig.awready;
    awid = sig.awid;
    awaddr = sig.awaddr;
    awlen = sig.awlen;
    awsize = sig.awsize;
    awburst = sig.awburst;
    awuser = sig.awuser;
    // Write data channel
    wvalid = sig.wvalid;
    wready = sig.wready;
    wid = sig.wid;
    wdata = sig.wdata;
    wlast = sig.wlast;
    wuser = sig.wuser;
    // Write response channel
    bvalid = sig.bvalid;
    bready = sig.bready;
    bid = sig.bid;
    bresp = sig.bresp;
    buser = sig.buser;
    // Read address channel
    arvalid = sig.arvalid;
    arready = sig.arready;
    arid = sig.arid;
    araddr = sig.araddr;
    arlen = sig.arlen;
    arsize = sig.arsize;
    arburst = sig.arburst;
    aruser = sig.aruser;
    // Read data channel
    rvalid = sig.rvalid;
    rready = sig.rready;
    rid = sig.rid;
    rdata = sig.rdata;
    rresp = sig.rresp;
    rlast = sig.rlast;
    ruser = sig.ruser;
  }
  aximm_signal(const sc_module_name &name)
      : awvalid(), awready(), awid(), awaddr(), awlen(), awsize(), awburst(),
        awuser(), wvalid(), wready(), wid(), wdata(), wlast(), wuser(),
        bvalid(), bready(), bid(), bresp(), buser(), arvalid(), arready(),
        arid(), araddr(), arlen(), arsize(), arburst(), aruser(), rvalid(),
        rready(), rid(), rdata(), rresp(), rlast(), ruser() {
    (void)name;
  }

  // Helper function for connecting the AXI-MM master and slave ports of two
  // modules
  void Connect(aximm_master_port &m, aximm_slave_port &s) {
    // Connect signal to master port
    m.awvalid(awvalid);
    m.awready(awready);
    m.awid(awid);
    m.awaddr(awaddr);
    m.awlen(awlen);
    m.awsize(awsize);
    m.awburst(awburst);
    m.awuser(awuser);
    m.wvalid(wvalid);
    m.wready(wready);
    m.wid(wid);
    m.wdata(wdata);
    m.wlast(wlast);
    m.wuser(wuser);
    m.bvalid(bvalid);
    m.bready(bready);
    m.bid(bid);
    m.bresp(bresp);
    m.buser(buser);
    m.arvalid(arvalid);
    m.arready(arready);
    m.arid(arid);
    m.araddr(araddr);
    m.arlen(arlen);
    m.arsize(arsize);
    m.arburst(arburst);
    m.aruser(aruser);
    m.rvalid(rvalid);
    m.rready(rready);
    m.rid(rid);
    m.rdata(rdata);
    m.rresp(rresp);
    m.rlast(rlast);
    m.ruser(ruser);
    // Connect signal to slave port
    s.awvalid(awvalid);
    s.awready(awready);
    s.awid(awid);
    s.awaddr(awaddr);
    s.awlen(awlen);
    s.awsize(awsize);
    s.awburst(awburst);
    s.awuser(awuser);
    s.wvalid(wvalid);
    s.wready(wready);
    s.wid(wid);
    s.wdata(wdata);
    s.wlast(wlast);
    s.wuser(wuser);
    s.bvalid(bvalid);
    s.bready(bready);
    s.bid(bid);
    s.bresp(bresp);
    s.buser(buser);
    s.arvalid(arvalid);
    s.arready(arready);
    s.arid(arid);
    s.araddr(araddr);
    s.arlen(arlen);
    s.arsize(arsize);
    s.arburst(arburst);
    s.aruser(aruser);
    s.rvalid(rvalid);
    s.rready(rready);
    s.rid(rid);
    s.rdata(rdata);
    s.rresp(rresp);
    s.rlast(rlast);
    s.ruser(ruser);
  }

  void Reset() {
    awvalid.write(false);
    awready.write(false);
    awid.write(0);
    awaddr.write(0);
    awlen.write(0);
    awsize.write(0);
    awburst.write(0);
    awuser.write(0);
    wvalid.write(false);
    wready.write(false);
    wid.write(0);
    wdata.write(0);
    wlast.write(false);
    wuser.write(0);
    bvalid.write(false);
    bready.write(false);
    bid.write(0);
    bresp.write(0);
    buser.write(0);
    arvalid.write(false);
    arready.write(false);
    arid.write(0);
    araddr.write(0);
    arlen.write(0);
    arsize.write(0);
    arburst.write(0);
    aruser.write(0);
    rvalid.write(false);
    rready.write(false);
    rid.write(0);
    rdata.write(0);
    rresp.write(0);
    rlast.write(false);
    ruser.write(0);
  }
};