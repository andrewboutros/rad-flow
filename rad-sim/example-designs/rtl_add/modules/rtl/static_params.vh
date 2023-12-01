// static_params.vh

`ifndef _static_params_vh_
`define _static_params_vh_

`define DATAW 128
`define FIFO_DEPTH 8

`define NOC_LINKS_DEST_WIDTH 4
`define NOC_LINKS_PACKETID_WIDTH 32

`define AXIS_STRBW 8
`define AXIS_KEEPW 8
`define AXIS_IDW `NOC_LINKS_PACKETID_WIDTH
`define AXIS_DESTW `NOC_LINKS_DEST_WIDTH
`define AXIS_USERW 75
`define AXIS_MAX_DATAW 512

`endif //_static_params_vh_
