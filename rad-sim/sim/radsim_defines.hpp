#pragma once

#define RADSIM_ROOT_DIR "/home/andrew/repos/rad-flow-dev/rad-sim"

// NoC-related Parameters
#define NOC_LINKS_PAYLOAD_WIDTH 166
#define NOC_LINKS_VCID_WIDTH 3
#define NOC_LINKS_PACKETID_WIDTH 32
#define NOC_LINKS_TYPEID_WIDTH 3
#define PACKET_TYPE_0 Flit::READ_REQUEST
#define PACKET_TYPE_1 Flit::WRITE_REQUEST
#define PACKET_TYPE_2 Flit::WRITE_DATA
#define PACKET_TYPE_3 Flit::READ_REPLY
#define PACKET_TYPE_4 Flit::WRITE_REPLY
#define NOC_LINKS_DEST_WIDTH 4
#define NOC_LINKS_DEST_INTERFACE_WIDTH 5

// AXI-Streaming Parameters
#define AXIS_MAX_DATAW 640
#define AXIS_STRBW 8
#define AXIS_KEEPW 8
#define AXIS_USERW 65

// Deduced Parameters
#define NOC_LINKS_WIDTH                                                        \
  (NOC_LINKS_PAYLOAD_WIDTH + NOC_LINKS_VCID_WIDTH + NOC_LINKS_PACKETID_WIDTH + \
   NOC_LINKS_DEST_WIDTH + NOC_LINKS_DEST_INTERFACE_WIDTH)
#define AXIS_IDW NOC_LINKS_DEST_INTERFACE_WIDTH
#define AXIS_DESTW NOC_LINKS_DEST_WIDTH
#define AXIS_TRANSACTION_WIDTH                                                 \
  (AXIS_MAX_DATAW + AXIS_STRBW + AXIS_KEEPW + AXIS_IDW + AXIS_DESTW +          \
   AXIS_USERW + 1)
#define AXIS_TRANSACTION_PAYLOAD_WIDTH                                         \
  (AXIS_IDW + AXIS_MAX_DATAW + AXIS_USERW + 1)
#define AXIS_TID(t) t.range(AXIS_IDW - 1, 0)
#define AXIS_TLAST(t) t.range(AXIS_IDW, AXIS_IDW)
#define AXIS_TUSER(t) t.range(AXIS_USERW + AXIS_IDW, AXIS_IDW + 1)
#define AXIS_TDATA(t)                                                          \
  t.range(AXIS_MAX_DATAW + AXIS_USERW + AXIS_IDW, AXIS_USERW + AXIS_IDW + 1)
#define AXIS_TDEST(t)                                                          \
  t.range(AXIS_DESTW + AXIS_MAX_DATAW + AXIS_USERW + AXIS_IDW,                 \
          AXIS_MAX_DATAW + AXIS_USERW + AXIS_IDW + 1)
#define AXIS_PAYLOAD(t) t.range(AXIS_IDW + AXIS_MAX_DATAW + AXIS_USERW, 0)

// AXI-MM Parameters
#define AXI_IDW 8
#define AXI_USERW 64
#define AXI_MAX_DATAW 512

// AXI-MM Constants
#define AXI_ADDRW 64
#define AXI_LENW 8
#define AXI_SIZEW 3
#define AXI_BURSTW 2
#define AXI_RESPW 2
#define AXI_CTRLW (AXI_LENW + AXI_SIZEW + AXI_BURSTW)
#define AXI_TRANSACTION_MAX_WIDTH                                              \
  (AXI_MAX_DATAW + AXI_RESPW + 1 + AXI_IDW + AXI_USERW)
#define AXI_USER(t) t.range(AXI_USERW - 1, 0)
#define AXI_CTRL(t) t.range(AXI_USERW + AXI_CTRLW - 1, AXI_USERW)
#define AXI_ADDR(t)                                                            \
  t.range(AXI_USERW + AXI_CTRLW + AXI_ADDRW - 1, AXI_USERW + AXI_CTRLW)
#define AXI_RESP(t) t.range(AXI_USERW + AXI_RESPW - 1, AXI_USERW)
#define AXI_LAST(t) t.range(AXI_USERW + AXI_RESPW, AXI_USERW + AXI_RESPW)
#define AXI_DATA(t)                                                            \
  t.range(AXI_USERW + AXI_RESPW + AXI_MAX_DATAW, AXI_USERW + AXI_RESPW + 1)

#define AXI_TYPE_AR 0
#define AXI_TYPE_AW 1
#define AXI_TYPE_W 2
#define AXI_TYPE_R 3
#define AXI_TYPE_B 4
#define AXI_NUM_RSP_TYPES 2
#define AXI_NUM_REQ_TYPES 3