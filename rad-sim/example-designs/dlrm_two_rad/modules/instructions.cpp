#include <instructions.hpp>

mvm_inst::mvm_inst()
    : en(false), jump(false), reduce(false), accum(0), accum_en(false),
      release(false), raddr(0), last(false), dest_layer(-1), dest_mvm(0) {}

bool mvm_inst::operator==(const mvm_inst &rhs) {
  return (en == rhs.en) && (jump == rhs.jump) && (reduce == rhs.reduce) &&
         (accum == rhs.accum) && (accum_en == rhs.accum_en) &&
         (release == rhs.release) && (raddr == rhs.raddr) &&
         (last == rhs.last) && (dest_layer == rhs.dest_layer) &&
         (dest_mvm == rhs.dest_mvm);
}

void mvm_inst::from_bv(const sc_bv<AXIS_MAX_DATAW> &inst_bv) {
  this->en = inst_bv.range(0, 0).to_uint();
  this->jump = inst_bv.range(1, 1).to_uint();
  this->reduce = inst_bv.range(2, 2).to_uint();
  this->accum = inst_bv.range(11, 3).to_uint();
  this->accum_en = inst_bv.range(12, 12).to_uint();
  this->release = inst_bv.range(13, 13).to_uint();
  this->raddr = inst_bv.range(22, 14).to_uint();
  this->last = inst_bv.range(23, 23).to_uint();
  this->dest_layer = inst_bv.range(28, 24).to_int();
  this->dest_mvm = inst_bv.range(33, 29).to_uint();
}

sc_bv<AXIS_MAX_DATAW> mvm_inst::to_bv() {
  sc_bv<AXIS_MAX_DATAW> inst_bv;
  inst_bv.range(0, 0) = this->en;
  inst_bv.range(1, 1) = this->jump;
  inst_bv.range(2, 2) = this->reduce;
  inst_bv.range(11, 3) = this->accum;
  inst_bv.range(12, 12) = this->accum_en;
  inst_bv.range(13, 13) = this->release;
  inst_bv.range(22, 14) = this->raddr;
  inst_bv.range(23, 23) = this->last;
  inst_bv.range(28, 24) = this->dest_layer;
  inst_bv.range(33, 29) = this->dest_mvm;
  return inst_bv;
}

ostream &operator<<(ostream &o, const mvm_inst &inst) {
  o << "{ en:" << inst.en << " jump:" << inst.jump << " reduce:" << inst.reduce
    << " accum:" << inst.accum << " accum_en:" << inst.accum_en
    << " release:" << inst.release << " raddr:" << inst.raddr
    << " last:" << inst.last << " dest_layer:" << inst.dest_layer
    << " dest_mvm:" << inst.dest_mvm << " }";
  return o;
}

void sc_trace(sc_trace_file *f, const mvm_inst &inst, const std::string &s) {
  sc_trace(f, inst.en, s + "_inst_en");
  sc_trace(f, inst.jump, s + "_inst_jump");
  sc_trace(f, inst.reduce, s + "_inst_reduce");
  sc_trace(f, inst.accum, s + "_inst_accum");
  sc_trace(f, inst.accum_en, s + "_inst_accum_en");
  sc_trace(f, inst.release, s + "_inst_release");
  sc_trace(f, inst.raddr, s + "_inst_raddr");
  sc_trace(f, inst.last, s + "_inst_last");
  sc_trace(f, inst.dest_layer, s + "_dest_layer");
  sc_trace(f, inst.dest_mvm, s + "_dest_mvm");
}