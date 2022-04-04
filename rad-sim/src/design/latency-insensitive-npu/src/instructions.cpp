#include <instructions.hpp>

mvu_uop::mvu_uop()
    : vrf_addr(0),
      vrf_rd_id(0),
      reg_sel(false),
      mrf_addr(0),
      tag(0),
      accum_op(0),
      accum_size(0),
      vrf_en(false),
      first_uop(false),
      last_uop(false) {}

bool mvu_uop::operator==(const mvu_uop& rhs) {
  return (vrf_addr == rhs.vrf_addr) && (vrf_rd_id == rhs.vrf_rd_id) && (reg_sel == rhs.reg_sel) &&
         (mrf_addr == rhs.mrf_addr) && (tag == rhs.tag) && (accum_op == rhs.accum_op) &&
         (accum_size == rhs.accum_size) && (vrf_en == rhs.vrf_en) && (first_uop == rhs.first_uop) &&
         (last_uop == rhs.last_uop);
}

ostream& operator<<(ostream& o, const mvu_uop& uop) {
  o << "{vaddr:" << uop.vrf_addr << " vid:" << uop.vrf_rd_id << " rsel:" << uop.reg_sel << " maddr:" << uop.mrf_addr
    << " tag:" << uop.tag << " accop:" << uop.accum_op << " accsz:" << uop.accum_size << " ven:" << uop.vrf_en << "}";
  return o;
}

void sc_trace(sc_trace_file* f, const mvu_uop& uop, const std::string& s) {
  sc_trace(f, uop.vrf_addr, s + "_vrf_addr");
  sc_trace(f, uop.vrf_rd_id, s + "_vrf_rd_id");
  sc_trace(f, uop.reg_sel, s + "_reg_sel");
  sc_trace(f, uop.mrf_addr, s + "_mrf_addr");
  sc_trace(f, uop.tag, s + "_tag");
  sc_trace(f, uop.accum_op, s + "_accum_op");
  sc_trace(f, uop.accum_size, s + "_accum_size");
  sc_trace(f, uop.vrf_en, s + "_vrf_en");
  sc_trace(f, uop.first_uop, s + "_first_uop");
  sc_trace(f, uop.last_uop, s + "_last_uop");
}

evrf_uop::evrf_uop() : vrf_addr(0), src_sel(0), tag(0), first_uop(false), last_uop(false) {}

bool evrf_uop::operator==(const evrf_uop& rhs) {
  return (vrf_addr == rhs.vrf_addr) && (src_sel == rhs.src_sel) && (tag == rhs.tag) && (first_uop == rhs.first_uop) &&
         (last_uop == rhs.last_uop);
}

ostream& operator<<(ostream& o, const evrf_uop& uop) {
  o << "{vaddr:" << uop.vrf_addr << " srcsel:" << uop.src_sel << " tag:" << uop.tag << "}";
  return o;
}

void sc_trace(sc_trace_file* f, const evrf_uop& uop, const std::string& s) {
  sc_trace(f, uop.vrf_addr, s + "_vrf_addr");
  sc_trace(f, uop.src_sel, s + "_src_sel");
  sc_trace(f, uop.tag, s + "_tag");
  sc_trace(f, uop.first_uop, s + "_first_uop");
  sc_trace(f, uop.last_uop, s + "_last_uop");
}

mfu_uop::mfu_uop()
    : vrf0_addr(0), vrf1_addr(0), tag(0), activation_op(0), add_op(0), mult_op(0), first_uop(false), last_uop(false) {}

bool mfu_uop::operator==(const mfu_uop& rhs) {
  return (vrf0_addr == rhs.vrf0_addr) && (vrf1_addr == rhs.vrf1_addr) && (tag == rhs.tag) &&
         (activation_op == rhs.activation_op) && (add_op == rhs.add_op) && (mult_op == rhs.mult_op) &&
         (first_uop == rhs.first_uop) && (last_uop == rhs.last_uop);
}

ostream& operator<<(ostream& o, const mfu_uop& uop) {
  o << "{vaaddr:" << uop.vrf0_addr << " vmaddr:" << uop.vrf1_addr << " tag:" << uop.tag << " act:" << uop.activation_op
    << " add:" << uop.add_op << " mult:" << uop.mult_op << "}";
  return o;
}

void sc_trace(sc_trace_file* f, const mfu_uop& uop, const std::string& s) {
  sc_trace(f, uop.vrf0_addr, s + "_vrf0_addr");
  sc_trace(f, uop.vrf1_addr, s + "_vrf1_addr");
  sc_trace(f, uop.tag, s + "_tag");
  sc_trace(f, uop.activation_op, s + "_activation_op");
  sc_trace(f, uop.add_op, s + "_add_op");
  sc_trace(f, uop.mult_op, s + "_mult_op");
  sc_trace(f, uop.first_uop, s + "_first_uop");
  sc_trace(f, uop.last_uop, s + "_last_uop");
}

ld_uop::ld_uop()
    : vrf0_id(0),
      block1_id(0),
      vrf1_id(0),
      vrf0_addr(0),
      vrf1_addr(0),
      src_sel(0),
      last(false),
      interrupt(false),
      output_result(false),
      first_uop(false),
      last_uop(false) {}

bool ld_uop::operator==(const ld_uop& rhs) {
  return (vrf0_id == rhs.vrf0_id) && (block1_id == rhs.block1_id) && (vrf1_id == rhs.vrf1_id) &&
         (vrf0_addr == rhs.vrf0_addr) && (vrf1_addr == rhs.vrf1_addr) && (src_sel == rhs.src_sel) &&
         (last == rhs.last) && (interrupt == rhs.interrupt) && (output_result == rhs.output_result) &&
         (first_uop == rhs.first_uop) && (last_uop == rhs.last_uop);
}

ostream& operator<<(ostream& o, const ld_uop& uop) {
  o << "{vid0:" << uop.vrf0_id << " blk1:" << uop.block1_id << " vid1:" << uop.vrf1_id << " v0addr:" << uop.vrf0_addr
    << " v1addr:" << uop.vrf1_addr << " srcsel:" << uop.src_sel << " last:" << uop.last
    << " interrupt:" << uop.interrupt << " res:" << uop.output_result << "}";
  return o;
}

void sc_trace(sc_trace_file* f, const ld_uop& uop, const std::string& s) {
  sc_trace(f, uop.vrf0_id, s + "_vrf0_id");
  sc_trace(f, uop.block1_id, s + "_block1_id");
  sc_trace(f, uop.vrf1_id, s + "_vrf1_id");
  sc_trace(f, uop.vrf0_addr, s + "_vrf0_addr");
  sc_trace(f, uop.vrf1_addr, s + "_vrf1_addr");
  sc_trace(f, uop.src_sel, s + "_src_sel");
  sc_trace(f, uop.last, s + "_last");
  sc_trace(f, uop.interrupt, s + "_interrupt");
  sc_trace(f, uop.output_result, s + "_output_result");
  sc_trace(f, uop.first_uop, s + "_first_uop");
  sc_trace(f, uop.last_uop, s + "_last_uop");
}

mvu_mop::mvu_mop()
    : vrf_base(TB_NUM_DOTS, 0), vrf_size(0), mrf_base(0), mrf_size(0), words_per_row(0), tag(0), op(false) {}

mvu_mop::mvu_mop(const mvu_mop& mop) : error_code(mop) {
  vrf_base.resize(TB_NUM_DOTS);
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) vrf_base[i] = mop.vrf_base[i];
  vrf_size = mop.vrf_size;
  mrf_base = mop.mrf_base;
  mrf_size = mop.mrf_size;
  words_per_row = mop.words_per_row;
  tag = mop.tag;
  op = mop.op;
}

mvu_mop& mvu_mop::operator=(const mvu_mop& mop) {
  vrf_base.resize(TB_NUM_DOTS);
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) vrf_base[i] = mop.vrf_base[i];
  vrf_size = mop.vrf_size;
  mrf_base = mop.mrf_base;
  mrf_size = mop.mrf_size;
  words_per_row = mop.words_per_row;
  tag = mop.tag;
  op = mop.op;
  return *this;
}

bool mvu_mop::operator==(const mvu_mop& rhs) {
  bool flag = true;
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) flag = flag && (vrf_base[i] == rhs.vrf_base[i]);
  return flag && (vrf_size == rhs.vrf_size) && (mrf_base == rhs.mrf_base) && (mrf_size == rhs.mrf_size) &&
         (words_per_row == rhs.words_per_row) && (tag == rhs.tag) && (op == rhs.op);
}

ostream& operator<<(ostream& o, const mvu_mop& mop) {
  o << "{";
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) o << "vbase" + std::to_string(i) << ":" << mop.vrf_base[i] << " ";
  o << "vsize:" << mop.vrf_size << " mbase:" << mop.mrf_base << " msize:" << mop.mrf_size
    << " wpr:" << mop.words_per_row << " tag:" << mop.tag << " op:" << mop.op << "}";
  return o;
}

sc_bv<MVU_MOP_BITWIDTH> mvu_mop::convert_to_bv() {
  sc_bv<MVU_MOP_BITWIDTH> mop_bv;
  unsigned int start_idx = 0;
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    mop_bv.range(start_idx + VRF_ADDRW - 1, start_idx) = vrf_base[i];
    start_idx += VRF_ADDRW;
  }
  mop_bv.range(start_idx + NSIZEW - 1, start_idx) = vrf_size;
  start_idx += NSIZEW;
  mop_bv.range(start_idx + MRF_ADDRW - 1, start_idx) = mrf_base;
  start_idx += MRF_ADDRW;
  mop_bv.range(start_idx + NSIZEW - 1, start_idx) = mrf_size;
  start_idx += NSIZEW;
  mop_bv.range(start_idx + NSIZEW - 1, start_idx) = words_per_row;
  start_idx += NSIZEW;
  mop_bv.range(start_idx + TAGW - 1, start_idx) = tag;
  start_idx += TAGW;
  mop_bv.range(start_idx, start_idx) = op;
  return mop_bv;
}

void mvu_mop::load_from_bv(sc_bv<MVU_MOP_BITWIDTH>& mop_bv) {
  unsigned int start_idx = 0;
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    vrf_base[i] = mop_bv.range(start_idx + VRF_ADDRW - 1, start_idx).to_uint();
    start_idx += VRF_ADDRW;
  }
  vrf_size = mop_bv.range(start_idx + NSIZEW - 1, start_idx).to_uint();
  start_idx += NSIZEW;
  mrf_base = mop_bv.range(start_idx + MRF_ADDRW - 1, start_idx).to_uint();
  start_idx += MRF_ADDRW;
  mrf_size = mop_bv.range(start_idx + NSIZEW - 1, start_idx).to_uint();
  start_idx += NSIZEW;
  words_per_row = mop_bv.range(start_idx + NSIZEW - 1, start_idx).to_uint();
  start_idx += NSIZEW;
  tag = mop_bv.range(start_idx + TAGW - 1, start_idx).to_uint();
  start_idx += TAGW;
  op = (bool)mop_bv.range(start_idx, start_idx).to_uint();
}

evrf_mop::evrf_mop() : vrf_base(TB_NUM_DOTS, 0), vrf_size(0), src_sel(false), tag(0), op(false), batch(0) {}

evrf_mop::evrf_mop(const evrf_mop& mop) : error_code(mop) {
  vrf_base.resize(TB_NUM_DOTS);
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) vrf_base[i] = mop.vrf_base[i];
  vrf_size = mop.vrf_size;
  src_sel = mop.src_sel;
  tag = mop.tag;
  op = mop.op;
  batch = mop.batch;
}

evrf_mop& evrf_mop::operator=(const evrf_mop& mop) {
  vrf_base.resize(TB_NUM_DOTS);
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) vrf_base[i] = mop.vrf_base[i];
  vrf_size = mop.vrf_size;
  src_sel = mop.src_sel;
  tag = mop.tag;
  op = mop.op;
  batch = mop.batch;
  return *this;
}

bool evrf_mop::operator==(const evrf_mop& rhs) {
  bool flag = true;
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) flag = flag && (vrf_base[i] == rhs.vrf_base[i]);
  return flag && (vrf_size == rhs.vrf_size) && (src_sel == rhs.src_sel) && (tag == rhs.tag) && (op == rhs.op) &&
         (batch == rhs.batch);
}

ostream& operator<<(ostream& o, const evrf_mop& mop) {
  o << "{";
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) o << "vbase" + std::to_string(i) << ":" << mop.vrf_base[i] << " ";
  o << "vsize:" << mop.vrf_size << " srcsel:" << mop.src_sel << " tag:" << mop.tag << " op:" << mop.op
    << " batch:" << mop.batch << "}";
  return o;
}

sc_bv<EVRF_MOP_BITWIDTH> evrf_mop::convert_to_bv() {
  sc_bv<EVRF_MOP_BITWIDTH> mop_bv;
  unsigned int start_idx = 0;
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    mop_bv.range(start_idx + VRF_ADDRW - 1, start_idx) = vrf_base[i];
    start_idx += VRF_ADDRW;
  }
  mop_bv.range(start_idx + NSIZEW - 1, start_idx) = vrf_size;
  start_idx += NSIZEW;
  mop_bv.range(start_idx, start_idx) = src_sel;
  start_idx += 1;
  mop_bv.range(start_idx + TAGW - 1, start_idx) = tag;
  start_idx += TAGW;
  mop_bv.range(start_idx, start_idx) = op;
  start_idx += 1;
  mop_bv.range(start_idx + 1, start_idx) = batch;
  return mop_bv;
}

void evrf_mop::load_from_bv(sc_bv<EVRF_MOP_BITWIDTH>& mop_bv) {
  unsigned int start_idx = 0;
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    vrf_base[i] = mop_bv.range(start_idx + VRF_ADDRW - 1, start_idx).to_uint();
    start_idx += VRF_ADDRW;
  }
  vrf_size = mop_bv.range(start_idx + NSIZEW - 1, start_idx).to_uint();
  start_idx += NSIZEW;
  src_sel = (bool)mop_bv.range(start_idx, start_idx).to_uint();
  start_idx += 1;
  tag = mop_bv.range(start_idx + TAGW - 1, start_idx).to_uint();
  start_idx += TAGW;
  op = (bool)mop_bv.range(start_idx, start_idx).to_uint();
  start_idx += 1;
  batch = mop_bv.range(start_idx + 1, start_idx).to_uint();
}

mfu_mop::mfu_mop() : add_vrf_base(TB_NUM_DOTS, 0), mul_vrf_base(TB_NUM_DOTS, 0), vrf_size(0), tag(0), op(0), batch(0) {}

mfu_mop::mfu_mop(const mfu_mop& mop) : error_code(mop) {
  add_vrf_base.resize(TB_NUM_DOTS);
  mul_vrf_base.resize(TB_NUM_DOTS);
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    add_vrf_base[i] = mop.add_vrf_base[i];
    mul_vrf_base[i] = mop.mul_vrf_base[i];
  }
  vrf_size = mop.vrf_size;
  tag = mop.tag;
  op = mop.op;
  batch = mop.batch;
}

mfu_mop& mfu_mop::operator=(const mfu_mop& mop) {
  add_vrf_base.resize(TB_NUM_DOTS);
  mul_vrf_base.resize(TB_NUM_DOTS);
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    add_vrf_base[i] = mop.add_vrf_base[i];
    mul_vrf_base[i] = mop.mul_vrf_base[i];
  }
  vrf_size = mop.vrf_size;
  tag = mop.tag;
  op = mop.op;
  batch = mop.batch;
  return *this;
}

bool mfu_mop::operator==(const mfu_mop& rhs) {
  bool flag = true;
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    flag = flag && (add_vrf_base[i] == rhs.add_vrf_base[i]);
    flag = flag && (mul_vrf_base[i] == rhs.mul_vrf_base[i]);
  }
  return flag && (vrf_size == rhs.vrf_size) && (tag == rhs.tag) && (op == rhs.op) && (batch == rhs.batch);
}

ostream& operator<<(ostream& o, const mfu_mop& mop) {
  o << "{";
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) o << "vabase" + std::to_string(i) << ":" << mop.add_vrf_base[i] << " ";
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) o << "vmbase" + std::to_string(i) << ":" << mop.mul_vrf_base[i] << " ";
  o << "vsize:" << mop.vrf_size << " tag:" << mop.tag << " op:" << mop.op << " batch:" << mop.batch << "}";
  return o;
}

sc_bv<MFU_MOP_BITWIDTH> mfu_mop::convert_to_bv() {
  sc_bv<MFU_MOP_BITWIDTH> mop_bv;
  unsigned int start_idx = 0;
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    mop_bv.range(start_idx + VRF_ADDRW - 1, start_idx) = add_vrf_base[i];
    start_idx += VRF_ADDRW;
  }
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    mop_bv.range(start_idx + VRF_ADDRW - 1, start_idx) = mul_vrf_base[i];
    start_idx += VRF_ADDRW;
  }
  mop_bv.range(start_idx + NSIZEW - 1, start_idx) = vrf_size;
  start_idx += NSIZEW;
  mop_bv.range(start_idx + TAGW - 1, start_idx) = tag;
  start_idx += TAGW;
  mop_bv.range(start_idx + 6, start_idx) = op;
  start_idx += 7;
  mop_bv.range(start_idx + 1, start_idx) = batch;
  return mop_bv;
}

void mfu_mop::load_from_bv(sc_bv<MFU_MOP_BITWIDTH>& mop_bv) {
  unsigned int start_idx = 0;
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    add_vrf_base[i] = mop_bv.range(start_idx + VRF_ADDRW - 1, start_idx).to_uint();
    start_idx += VRF_ADDRW;
  }
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    mul_vrf_base[i] = mop_bv.range(start_idx + VRF_ADDRW - 1, start_idx).to_uint();
    start_idx += VRF_ADDRW;
  }
  vrf_size = mop_bv.range(start_idx + NSIZEW - 1, start_idx).to_uint();
  start_idx += NSIZEW;
  tag = mop_bv.range(start_idx + TAGW - 1, start_idx).to_uint();
  start_idx += TAGW;
  op = mop_bv.range(start_idx + 6, start_idx).to_uint();
  start_idx += 7;
  batch = mop_bv.range(start_idx + 1, start_idx).to_uint();
}

ld_mop::ld_mop()
    : vrf0_id(0),
      block1_id(0),
      vrf1_id(0),
      vrf0_base(TB_NUM_DOTS, 0),
      vrf1_base(TB_NUM_DOTS, 0),
      vrf_size(0),
      src_sel(0),
      op(false),
      batch(0),
      interrupt(false),
      output_result(false) {}

ld_mop::ld_mop(const ld_mop& mop) : error_code(mop) {
  vrf0_id = mop.vrf0_id;
  block1_id = mop.block1_id;
  vrf1_id = mop.vrf1_id;
  vrf0_base.resize(TB_NUM_DOTS);
  vrf1_base.resize(TB_NUM_DOTS);
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    vrf0_base[i] = mop.vrf0_base[i];
    vrf1_base[i] = mop.vrf1_base[i];
  }
  vrf_size = mop.vrf_size;
  src_sel = mop.src_sel;
  op = mop.op;
  batch = mop.batch;
  interrupt = mop.interrupt;
  output_result = mop.output_result;
}

ld_mop& ld_mop::operator=(const ld_mop& mop) {
  vrf0_id = mop.vrf0_id;
  block1_id = mop.block1_id;
  vrf1_id = mop.vrf1_id;
  vrf0_base.resize(TB_NUM_DOTS);
  vrf1_base.resize(TB_NUM_DOTS);
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    vrf0_base[i] = mop.vrf0_base[i];
    vrf1_base[i] = mop.vrf1_base[i];
  }
  vrf_size = mop.vrf_size;
  src_sel = mop.src_sel;
  op = mop.op;
  batch = mop.batch;
  interrupt = mop.interrupt;
  output_result = mop.output_result;
  return *this;
}

bool ld_mop::operator==(const ld_mop& rhs) {
  bool flag = true;
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    flag = flag && (vrf0_base[i] == rhs.vrf0_base[i]);
    flag = flag && (vrf1_base[i] == rhs.vrf1_base[i]);
  }
  return (vrf0_id == rhs.vrf0_id) && (block1_id == rhs.block1_id) && (vrf1_id == rhs.vrf1_id) && flag &&
         (vrf_size == rhs.vrf_size) && (src_sel == rhs.src_sel) && (op == rhs.op) && (batch == rhs.batch) &&
         (interrupt == rhs.interrupt) && (output_result == rhs.output_result);
}

ostream& operator<<(ostream& o, const ld_mop& mop) {
  o << "{vid0:" << mop.vrf0_id << " blk1:" << mop.block1_id << " vid1:" << mop.vrf1_id << " ";
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) o << "v0base" + std::to_string(i) << ":" << mop.vrf0_base[i] << " ";
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) o << "v1base" + std::to_string(i) << ":" << mop.vrf1_base[i] << " ";
  o << "vsize:" << mop.vrf_size << " srcsel:" << mop.src_sel << " op:" << mop.op << " batch:" << mop.batch
    << " interrupt:" << mop.interrupt << " out:" << mop.output_result << "}";
  return o;
}

sc_bv<LD_MOP_BITWIDTH> ld_mop::convert_to_bv() {
  sc_bv<LD_MOP_BITWIDTH> mop_bv;
  unsigned int start_idx = 0;
  mop_bv.range(start_idx + VRF_WB_SELW - 1, start_idx) = vrf0_id;
  start_idx += VRF_WB_SELW;
  mop_bv.range(start_idx + BLOCK_WB_SELW - 1, start_idx) = block1_id;
  start_idx += BLOCK_WB_SELW;
  mop_bv.range(start_idx + VRF_WB_SELW - 1, start_idx) = vrf1_id;
  start_idx += VRF_WB_SELW;
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    mop_bv.range(start_idx + VRF_ADDRW - 1, start_idx) = vrf0_base[i];
    start_idx += VRF_ADDRW;
  }
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    mop_bv.range(start_idx + VRF_ADDRW - 1, start_idx) = vrf1_base[i];
    start_idx += VRF_ADDRW;
  }
  mop_bv.range(start_idx + NSIZEW - 1, start_idx) = vrf_size;
  start_idx += NSIZEW;
  mop_bv.range(start_idx + 1, start_idx) = src_sel;
  start_idx += 2;
  mop_bv.range(start_idx, start_idx) = op;
  start_idx += 1;
  mop_bv.range(start_idx + 1, start_idx) = batch;
  start_idx += 2;
  mop_bv.range(start_idx, start_idx) = interrupt;
  start_idx += 1;
  mop_bv.range(start_idx, start_idx) = output_result;
  return mop_bv;
}

void ld_mop::load_from_bv(sc_bv<LD_MOP_BITWIDTH>& mop_bv) {
  unsigned int start_idx = 0;
  vrf0_id = mop_bv.range(start_idx + VRF_WB_SELW - 1, start_idx).to_uint();
  start_idx += VRF_WB_SELW;
  block1_id = mop_bv.range(start_idx + BLOCK_WB_SELW - 1, start_idx).to_uint();
  start_idx += BLOCK_WB_SELW;
  vrf1_id = mop_bv.range(start_idx + VRF_WB_SELW - 1, start_idx).to_uint();
  start_idx += VRF_WB_SELW;
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    vrf0_base[i] = mop_bv.range(start_idx + VRF_ADDRW - 1, start_idx).to_uint();
    start_idx += VRF_ADDRW;
  }
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    vrf1_base[i] = mop_bv.range(start_idx + VRF_ADDRW - 1, start_idx).to_uint();
    start_idx += VRF_ADDRW;
  }
  vrf_size = mop_bv.range(start_idx + NSIZEW - 1, start_idx).to_uint();
  start_idx += NSIZEW;
  src_sel = mop_bv.range(start_idx + 1, start_idx).to_uint();
  start_idx += 2;
  op = (bool)mop_bv.range(start_idx, start_idx).to_uint();
  start_idx += 1;
  batch = mop_bv.range(start_idx + 1, start_idx).to_uint();
  start_idx += 2;
  interrupt = (bool)mop_bv.range(start_idx, start_idx).to_uint();
  start_idx += 1;
  output_result = (bool)mop_bv.range(start_idx, start_idx).to_uint();
}

vliw_inst::vliw_inst() : mvu_inst(), evrf_inst(), mfu0_inst(), mfu1_inst(), ld_inst() {}

vliw_inst::vliw_inst(const vliw_inst& inst) : error_code(inst) {
  mvu_inst = inst.mvu_inst;
  evrf_inst = inst.evrf_inst;
  mfu0_inst = inst.mfu0_inst;
  mfu1_inst = inst.mfu1_inst;
  ld_inst = inst.ld_inst;
}

vliw_inst& vliw_inst::operator=(const vliw_inst& inst) {
  mvu_inst = inst.mvu_inst;
  evrf_inst = inst.evrf_inst;
  mfu0_inst = inst.mfu0_inst;
  mfu1_inst = inst.mfu1_inst;
  ld_inst = inst.ld_inst;
  return *this;
}

bool vliw_inst::operator==(const vliw_inst& rhs) {
  return (mvu_inst == rhs.mvu_inst) && (evrf_inst == rhs.evrf_inst) && (mfu0_inst == rhs.mfu0_inst) &&
         (mfu1_inst == rhs.mfu1_inst) && (ld_inst == rhs.ld_inst);
}

ostream& operator<<(ostream& o, const vliw_inst& inst) {
  o << inst.mvu_inst << "\n";
  o << inst.evrf_inst << "\n";
  o << inst.mfu0_inst << "\n";
  o << inst.mfu1_inst << "\n";
  o << inst.ld_inst << "\n";
  return o;
}

void sc_trace(sc_trace_file* f, const mvu_mop& mop, const std::string& s) {
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) sc_trace(f, mop.vrf_base[i], s + "_vrf_base" + std::to_string(i));
  sc_trace(f, mop.vrf_size, s + "_vrf_size");
  sc_trace(f, mop.mrf_base, s + "_mrf_base");
  sc_trace(f, mop.mrf_size, s + "_mrf_size");
  sc_trace(f, mop.tag, s + "_tag");
  sc_trace(f, mop.words_per_row, s + "_words_per_row");
  sc_trace(f, mop.op, s + "_op");
}

void sc_trace(sc_trace_file* f, const evrf_mop& mop, const std::string& s) {
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) sc_trace(f, mop.vrf_base[i], s + "_vrf_base" + std::to_string(i));
  sc_trace(f, mop.vrf_size, s + "_vrf_size");
  sc_trace(f, mop.src_sel, s + "_src_sel");
  sc_trace(f, mop.tag, s + "_tag");
  sc_trace(f, mop.op, s + "_op");
  sc_trace(f, mop.batch, s + "_batch");
}

void sc_trace(sc_trace_file* f, const mfu_mop& mop, const std::string& s) {
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    sc_trace(f, mop.add_vrf_base[i], s + "_add_vrf_base" + std::to_string(i));
    sc_trace(f, mop.mul_vrf_base[i], s + "_mul_vrf_base" + std::to_string(i));
  }
  sc_trace(f, mop.vrf_size, s + "_vrf_size");
  sc_trace(f, mop.tag, s + "_tag");
  sc_trace(f, mop.op, s + "_op");
  sc_trace(f, mop.batch, s + "_batch");
}

void sc_trace(sc_trace_file* f, const ld_mop& mop, const std::string& s) {
  sc_trace(f, mop.vrf0_id, s + "_vrf0_id");
  sc_trace(f, mop.block1_id, s + "_block1_id");
  sc_trace(f, mop.vrf1_id, s + "_vrf1_id");
  for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
    sc_trace(f, mop.vrf0_base[i], s + "_vrf0_base" + std::to_string(i));
    sc_trace(f, mop.vrf1_base[i], s + "_vrf1_base" + std::to_string(i));
  }
  sc_trace(f, mop.vrf_size, s + "_vrf_size");
  sc_trace(f, mop.src_sel, s + "_src_sel");
  sc_trace(f, mop.op, s + "_op");
  sc_trace(f, mop.batch, s + "_batch");
  sc_trace(f, mop.interrupt, s + "_interrupt");
  sc_trace(f, mop.output_result, s + "_output_result");
}

void sc_trace(sc_trace_file* f, const vliw_inst& inst, const std::string& s) {
  sc_trace(f, inst.mvu_inst, s + "_mvu_inst");
  sc_trace(f, inst.evrf_inst, s + "_evrf_inst");
  sc_trace(f, inst.mfu0_inst, s + "_mfu0_inst");
  sc_trace(f, inst.mfu1_inst, s + "_mfu1_inst");
  sc_trace(f, inst.ld_inst, s + "_ld_inst");
}