#pragma once

#include <systemc.h>
#include "radsim_defines.hpp"

class mvm_inst : public std::error_code {
public:
    bool release_op;             // [31]
    sc_uint<9> release_dest;     // [30:22]
    sc_uint<9> rf_raddr;         // [21:13]
    sc_uint<9> accum_raddr;      // [12:4]
    bool last;                   // [3]
    bool release;                // [2]
    bool accum_en;               // [1]
    bool reduce;                 // [0]

    mvm_inst();    
    bool operator == (const mvm_inst &rhs);
    void from_bv(const sc_bv<AXIS_MAX_DATAW> &inst_bv);
    sc_bv<AXIS_MAX_DATAW> to_bv();
    friend ostream& operator << (ostream& o, const mvm_inst &inst); 
};
void sc_trace(sc_trace_file* f, const mvm_inst& inst, const std::string& s);