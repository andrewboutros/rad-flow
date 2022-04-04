#pragma once

#include <systemc.h>
#include <params.hpp>

#define ACCUM_OP_SET 0
#define ACCUM_OP_UPD 1
#define ACCUM_OP_WRB 2
#define ACCUM_OP_SWB 3
#define EVRF_FROM_MVU 0
#define EVRF_FROM_VRF 1
#define EVRF_FLUSH_MVU 2
#define MFU_RELU_OP 1
#define MFU_SIGMOID_OP 2
#define MFU_TANH_OP 3
#define MFU_ADD_OP 1
#define MFU_SUB_OP 2
#define MFU_RSUB_OP 3
#define MFU_MAX_OP 4
#define MFU_MUL_OP 1
#define LD_FROM_IN 0
#define LD_FROM_WB 1
#define LD_TAG_UPDATE 2

#define MVU_MOP_BITWIDTH (TB_NUM_DOTS*VRF_ADDRW+3*NSIZEW+MRF_ADDRW+TAGW+1)
#define EVRF_MOP_BITWIDTH (TB_NUM_DOTS*VRF_ADDRW+NSIZEW+TAGW+4)
#define MFU_MOP_BITWIDTH (2*TB_NUM_DOTS*VRF_ADDRW+NSIZEW+TAGW+9)
#define LD_MOP_BITWIDTH (2*VRF_WB_SELW+BLOCK_WB_SELW+2*TB_NUM_DOTS*VRF_ADDRW+NSIZEW+7)

class mvu_uop : public std::error_code {
public:
    sc_uint<VRF_ADDRW> vrf_addr;
    sc_uint<VRFIDW> vrf_rd_id;
    bool reg_sel;
    sc_uint<MRF_ADDRW> mrf_addr;
    sc_uint<TAGW> tag;
    sc_uint<2> accum_op;
    sc_uint<ACCUMIDW> accum_size;
    bool vrf_en;
    bool first_uop;
    bool last_uop;

    mvu_uop();    
    bool operator == (const mvu_uop &rhs);
    friend ostream& operator << ( ostream& o, const mvu_uop &uop); 
};
void sc_trace( sc_trace_file* f, const mvu_uop& uop, const std::string& s );

class evrf_uop : public std::error_code {
public:
    sc_uint<VRF_ADDRW> vrf_addr;
    sc_uint<2> src_sel;
    sc_uint<TAGW> tag;
    bool first_uop;
    bool last_uop;

    evrf_uop();
    bool operator== (const evrf_uop &rhs);
    friend ostream& operator << ( ostream& o, const evrf_uop &uop );
};
void sc_trace( sc_trace_file* f, const evrf_uop& uop, const std::string& s );

class mfu_uop : public std::error_code {
public:
    sc_uint<VRF_ADDRW> vrf0_addr;
    sc_uint<VRF_ADDRW> vrf1_addr;
    sc_uint<TAGW> tag;
    sc_uint<2> activation_op;
    sc_uint<3> add_op;
    sc_uint<1> mult_op;
    bool first_uop;
    bool last_uop;

    mfu_uop();    
    bool operator== (const mfu_uop &rhs);
    friend ostream& operator << ( ostream& o, const mfu_uop &uop );
};
void sc_trace( sc_trace_file* f, const mfu_uop& uop, const std::string& s );

class ld_uop : public std::error_code {
public:
    sc_uint<VRF_WB_SELW> vrf0_id;
    sc_uint<BLOCK_WB_SELW> block1_id;
    sc_uint<VRF_WB_SELW> vrf1_id;
    sc_uint<VRF_ADDRW> vrf0_addr;
    sc_uint<VRF_ADDRW> vrf1_addr;
    sc_uint<2> src_sel;
    bool last;
    bool interrupt;
    bool output_result;
    bool first_uop;
    bool last_uop;

    ld_uop();
    bool operator== (const ld_uop &rhs);
    friend ostream& operator << ( ostream& o, const ld_uop &uop ); 
};
void sc_trace( sc_trace_file* f, const ld_uop& uop, const std::string& s );

class mvu_mop : public std::error_code {
public:
    std::vector <sc_uint<VRF_ADDRW>> vrf_base;
    sc_uint<NSIZEW> vrf_size;
    sc_uint<MRF_ADDRW> mrf_base;
    sc_uint<NSIZEW> mrf_size;
    sc_uint<NSIZEW> words_per_row;
    sc_uint<TAGW> tag;
    bool op;

    mvu_mop();     
    mvu_mop(const mvu_mop &mop);
    mvu_mop& operator= (const mvu_mop &mop);
    bool operator== (const mvu_mop &rhs); 
    friend ostream& operator << ( ostream& o, const mvu_mop &mop );
    sc_bv<MVU_MOP_BITWIDTH> convert_to_bv();
    void load_from_bv(sc_bv<MVU_MOP_BITWIDTH>& mop_bv);

    mvu_mop(unsigned int i) { mvu_mop(); };
    sc_uint<TAGW>& operator[] (unsigned int idx) { return tag; };
    int size() { return 0; };
};
void sc_trace( sc_trace_file* f, const mvu_mop& mop, const std::string& s );

class evrf_mop : public std::error_code {
public:
    std::vector <sc_uint<VRF_ADDRW>> vrf_base;
    sc_uint<NSIZEW> vrf_size;
    bool src_sel;
    sc_uint<TAGW> tag;
    bool op;
    sc_uint<2> batch;

    evrf_mop(); 
    evrf_mop(const evrf_mop& mop);
    evrf_mop& operator= (const evrf_mop& mop);
    bool operator== (const evrf_mop &rhs);     
    friend ostream& operator<< ( ostream& o, const evrf_mop &mop );
    sc_bv<EVRF_MOP_BITWIDTH> convert_to_bv();
    void load_from_bv(sc_bv<EVRF_MOP_BITWIDTH>& mop_bv);

    evrf_mop(unsigned int i) { evrf_mop(); };
    sc_uint<TAGW>& operator[] (unsigned int idx) { return tag; };
    int size() { return 0; };
};
void sc_trace( sc_trace_file* f, const evrf_mop& mop, const std::string& s );

class mfu_mop : public std::error_code {
public:
    std::vector <sc_uint<VRF_ADDRW>> add_vrf_base;
    std::vector <sc_uint<VRF_ADDRW>> mul_vrf_base;
    sc_uint<NSIZEW> vrf_size;
    sc_uint<TAGW> tag;
    sc_uint<7> op;
    sc_uint<2> batch;

    mfu_mop();    
    mfu_mop(const mfu_mop &mop);    
    mfu_mop& operator= (const mfu_mop &mop);
    bool operator== (const mfu_mop &rhs);     
    friend ostream& operator<< ( ostream& o, const mfu_mop &mop );
    sc_bv<MFU_MOP_BITWIDTH> convert_to_bv();
    void load_from_bv(sc_bv<MFU_MOP_BITWIDTH>& mop_bv);

    mfu_mop(unsigned int i) { mfu_mop(); };
    sc_uint<TAGW>& operator[] (unsigned int idx) { return tag; };
    int size() { return 0; };
};
void sc_trace( sc_trace_file* f, const mfu_mop& mop, const std::string& s );

class ld_mop : public std::error_code {
public:
    sc_uint<VRF_WB_SELW> vrf0_id;
    sc_uint<BLOCK_WB_SELW> block1_id;
    sc_uint<VRF_WB_SELW> vrf1_id;
    std::vector <sc_uint<VRF_ADDRW>> vrf0_base;
    std::vector <sc_uint<VRF_ADDRW>> vrf1_base;
    sc_uint<NSIZEW> vrf_size;
    sc_uint<2> src_sel;
    bool op;
    sc_uint<2> batch;
    bool interrupt;
    bool output_result;

    ld_mop();    
    ld_mop(const ld_mop &mop); 
    ld_mop& operator= (const ld_mop &mop);
    bool operator == (const ld_mop &rhs); 
    friend ostream& operator << ( ostream& o, const ld_mop &mop ); 
    sc_bv<LD_MOP_BITWIDTH> convert_to_bv();
    void load_from_bv(sc_bv<LD_MOP_BITWIDTH>& mop_bv);

    ld_mop(unsigned int i) { ld_mop(); };
    sc_uint<2>& operator[] (unsigned int idx) { return src_sel; };
    int size() { return 0; };
};
void sc_trace( sc_trace_file* f, const ld_mop& mop, const std::string& s );

class vliw_inst : public std::error_code {
public:
    mvu_mop mvu_inst;
    evrf_mop evrf_inst;
    mfu_mop mfu0_inst;
    mfu_mop mfu1_inst;
    ld_mop ld_inst;

    vliw_inst();    
    vliw_inst(const vliw_inst &inst); 
    vliw_inst& operator= (const vliw_inst &inst);
    bool operator== (const vliw_inst &rhs); 
    friend ostream& operator<< ( ostream& o, const vliw_inst &inst );
};
void sc_trace( sc_trace_file* f, const vliw_inst& inst, const std::string& s );
