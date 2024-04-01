#include <instructions.hpp>

mvm_inst::mvm_inst()
    : release_op(false),
      release_dest(0),
      rf_raddr(0),
      accum_raddr(0),
      last(false),
      release(false),
      accum_en(false),
      reduce(false) {}

bool mvm_inst::operator==(const mvm_inst& rhs) {
  return (release_op == rhs.release_op) && (release_dest == rhs.release_dest) && (rf_raddr == rhs.rf_raddr) && 
         (accum_raddr == rhs.accum_raddr) && (last == rhs.last) && (release == rhs.release) &&
         (accum_en == rhs.accum_en) && (reduce == rhs.reduce);
}

void mvm_inst::from_bv(const sc_bv<AXIS_MAX_DATAW> &inst_bv) {
  this->reduce       = inst_bv.range( 0,  0).to_uint();
  this->accum_en     = inst_bv.range( 1,  1).to_uint();
  this->release      = inst_bv.range( 2,  2).to_uint();
  this->last         = inst_bv.range( 3,  3).to_uint();
  this->accum_raddr  = inst_bv.range(12,  4).to_uint();
  this->rf_raddr     = inst_bv.range(21, 13).to_uint();
  this->release_dest = inst_bv.range(30, 22).to_uint();
  this->release_op   = inst_bv.range(31, 31).to_uint();
}

sc_bv<AXIS_MAX_DATAW> mvm_inst::to_bv() {
  sc_bv<AXIS_MAX_DATAW> inst_bv;
  inst_bv.range( 0,  0) = this->reduce;
  inst_bv.range( 1,  1) = this->accum_en;
  inst_bv.range( 2,  2) = this->release;
  inst_bv.range( 3,  3) = this->last;
  inst_bv.range(12,  4) = this->accum_raddr;
  inst_bv.range(21, 13) = this->rf_raddr;
  inst_bv.range(30, 22) = this->release_dest;
  inst_bv.range(31, 31) = this->release_op;
  return inst_bv;
}

ostream& operator<<(ostream& o, const mvm_inst& inst) {
  o << "{ reduce:" << inst.reduce 
    << " accum_en:" << inst.accum_en 
    << " release:" << inst.release 
    << " last:" << inst.last
    << " accum_raddr:" << inst.accum_raddr
    << " rf_raddr:" << inst.rf_raddr 
    << " release_dest:" << inst.release_dest 
    << " release_op:" << inst.release_op << " }";
  return o;
}

void sc_trace(sc_trace_file* f, const mvm_inst& inst, const std::string& s) {
  sc_trace(f, inst.reduce, s + "_inst_reduce");
  sc_trace(f, inst.accum_en, s + "_inst_accum_en");
  sc_trace(f, inst.release, s + "_inst_release");
  sc_trace(f, inst.last, s + "_inst_last");
  sc_trace(f, inst.accum_raddr, s + "_inst_accum_raddr");
  sc_trace(f, inst.rf_raddr, s + "_inst_raddr");
  sc_trace(f, inst.release_dest, s + "_inst_release_dest");
  sc_trace(f, inst.release_op, s + "_inst_release_op");
}