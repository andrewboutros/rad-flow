#pragma once

#include <radsim_config.hpp>
#include <radsim_telemetry.hpp>

#define RADSIM_SRC_ROOTDIR "/Users/andrew/PhD/dev/rad-sim-opt-npu-multithread-hard-c2/src/"

// Clock Periods in Nanoseconds
#define NUM_NODE_CLKS 2
#define FABRIC_PERIOD 5.0
#define BASEDIE_PERIOD 2.5
#define NOC_PERIOD 1.0
#define ADAPTER_PERIOD 1.25

// NoC Architecture Parameters
#define SUBNETS 1
#define NOC_NODES 100
#define NOC_LINKS_PAYLOAD_WIDTH 166
#define NOC_LINKS_VCID_WIDTH 3
#define NOC_LINKS_PACKETID_WIDTH 32
#define NOC_LINKS_TYPEID_WIDTH 3
#define NOC_LINKS_DEST_WIDTH 8
#define NOC_LINKS_DEST_INTERFACE_WIDTH 5
#define NOC_LINKS_WIDTH                                                                               \
  (NOC_LINKS_PAYLOAD_WIDTH + NOC_LINKS_VCID_WIDTH + NOC_LINKS_PACKETID_WIDTH + NOC_LINKS_DEST_WIDTH + \
   NOC_LINKS_DEST_INTERFACE_WIDTH)

// AXI-Streaming Transaction Parameters & Macros
#define AXIS_MAX_DATAW 640
#define AXIS_STRBW 8
#define AXIS_KEEPW 8
#define AXIS_USERW 32
#define AXIS_IDW NOC_LINKS_DEST_INTERFACE_WIDTH
#define AXIS_DESTW NOC_LINKS_DEST_WIDTH
#define AXIS_TRANSACTION_WIDTH (AXIS_MAX_DATAW + AXIS_STRBW + AXIS_KEEPW + AXIS_IDW + AXIS_DESTW + AXIS_USERW + 1)
#define AXIS_TRANSACTION_PAYLOAD_WIDTH (AXIS_IDW + AXIS_MAX_DATAW + AXIS_USERW + 1)
#define AXIS_TID(t) t.range(AXIS_IDW - 1, 0)
#define AXIS_TLAST(t) t.range(AXIS_IDW, AXIS_IDW)
#define AXIS_TUSER(t) t.range(AXIS_USERW + AXIS_IDW, AXIS_IDW + 1)
#define AXIS_TDATA(t) t.range(AXIS_MAX_DATAW + AXIS_USERW + AXIS_IDW, AXIS_USERW + AXIS_IDW + 1)
#define AXIS_TDEST(t) t.range(AXIS_DESTW + AXIS_MAX_DATAW + AXIS_USERW + AXIS_IDW, AXIS_MAX_DATAW + AXIS_USERW + AXIS_IDW + 1)
#define AXIS_PAYLOAD(t) t.range(AXIS_IDW + AXIS_MAX_DATAW + AXIS_USERW, 0)

#define AXIS_TYPE0 Flit::READ_REQUEST
#define AXIS_TYPE1 Flit::WRITE_REQUEST
#define AXIS_TYPE2 Flit::WRITE_DATA
#define AXIS_TYPE3 Flit::READ_REPLY
#define AXIS_TYPE4 Flit::WRITE_REPLY

// Global Variables
extern RADSimConfig crossroads_config;
extern std::ofstream* radsim_log;
extern std::ostream* gWatchOut;
extern std::unordered_map<std::string, unsigned int> module_assignment;