#pragma once

#define DATAW 512
#define IPRECISION 8
#define OPRECISION 32
#define LANES (DATAW / IPRECISION)
#define DPES LANES
#define RF_DEPTH 512
#define RF_ADDRW 9
#define FIFOD 64
#define DATAPATH_DELAY 12
#define RF_RD_LATENCY 2
#define RF_WR_LATENCY 2