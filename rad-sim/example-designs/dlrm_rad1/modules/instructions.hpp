#pragma once

#include <radsim_defines.hpp>
#include <systemc.h>

class mvm_inst : public std::error_code {
public:
  bool en;
  bool jump;
  bool reduce;
  unsigned int accum;
  bool accum_en;
  bool release;
  unsigned int raddr;
  bool last;
  sc_int<5> dest_layer;
  sc_uint<5> dest_mvm;

  mvm_inst();
  bool operator==(const mvm_inst &rhs);
  void from_bv(const sc_bv<AXIS_MAX_DATAW> &inst_bv);
  sc_bv<AXIS_MAX_DATAW> to_bv();
  friend ostream &operator<<(ostream &o, const mvm_inst &inst);
};
void sc_trace(sc_trace_file *f, const mvm_inst &inst, const std::string &s);