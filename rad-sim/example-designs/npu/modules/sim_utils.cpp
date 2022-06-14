#include <sim_utils.hpp>
#include <axis_interface.hpp>

template <class dtype>
data_vector<dtype>::data_vector() {}

template <class dtype>
data_vector<dtype>::data_vector(unsigned int size) {
  v.resize(size);
}

template <class dtype>
data_vector<dtype>::data_vector(sc_vector<sc_in<dtype>>& iport) {
  v.resize(iport.size());
  for (unsigned int i = 0; i < iport.size(); i++) v[i] = iport[i].read();
}

template <class dtype>
data_vector<dtype>::data_vector(std::vector<dtype>& vec) {
  v.resize(vec.size());
  for (unsigned int i = 0; i < vec.size(); i++) v[i] = vec[i];
}

template <class dtype>
bool data_vector<dtype>::operator==(const data_vector<dtype>& rhs) {
  if (v.size() != rhs.v.size()) return false;
  bool is_equal = true;
  for (unsigned int i = 0; i < v.size(); i++) is_equal &= (v[i] == rhs.v[i]);
  return is_equal;
}

template <class dtype>
bool data_vector<dtype>::operator==(const std::vector<dtype>& rhs) {
  if (v.size() != rhs.size()) return false;
  bool is_equal = true;
  for (unsigned int i = 0; i < v.size(); i++) is_equal &= (v[i] == rhs[i]);
  return is_equal;
}

template <class dtype>
dtype& data_vector<dtype>::operator[](unsigned int idx) {
  return v[idx];
}

template <class dtype>
unsigned int data_vector<dtype>::size() {
  return v.size();
}

template <class dtype>
void data_vector<dtype>::data_vector_truncate(const data_vector<tb_output_precision>& v1) {
  v.resize(v1.v.size());
  for (unsigned int i = 0; i < v1.v.size(); i++) v[i] = v1.v[i];
}

template <class dtype>
ostream& operator<<(ostream& o, const data_vector<dtype>& dvector) {
  for (unsigned int i = 0; i < dvector.v.size(); i++) o << dvector.v[i] << " ";
  return o;
}
template ostream& operator<<<tb_input_precision>(ostream& o, const data_vector<tb_input_precision>& dvector);
template ostream& operator<<<tb_output_precision>(ostream& o, const data_vector<tb_output_precision>& dvector);

template <class dtype>
data_vector<dtype> operator+(const data_vector<dtype>& v1, const data_vector<dtype>& v2) {
  assert(v1.v.size() == v2.v.size());
  if (v1.v.size() != v2.v.size()) {
    cerr << "Attempting to add two data vectors of different sizes! ";
    cerr << "(" << v1.v.size() << "," << v2.v.size() << ")" << endl;
    exit(1);
  }

  data_vector<dtype> res = data_vector<dtype>(v1.v.size());
  for (unsigned int i = 0; i < v1.v.size(); i++) res.v[i] = v1.v[i] + v2.v[i];
  return res;
}
template data_vector<tb_input_precision> operator+
    <tb_input_precision>(const data_vector<tb_input_precision>& v1, const data_vector<tb_input_precision>& v2);
template data_vector<tb_output_precision> operator+
    <tb_output_precision>(const data_vector<tb_output_precision>& v1, const data_vector<tb_output_precision>& v2);

template <class dtype>
data_vector<dtype> operator-(const data_vector<dtype>& v1, const data_vector<dtype>& v2) {
  if (v1.v.size() != v2.v.size()) {
    cerr << "Attempting to subtract two data vectors of different sizes!";
    cerr << "(" << v1.v.size() << "," << v2.v.size() << ")" << endl;
    exit(1);
  }

  data_vector<dtype> res = data_vector<dtype>(v1.v.size());
  for (unsigned int i = 0; i < v1.v.size(); i++) res.v[i] = v1.v[i] - v2.v[i];
  return res;
}
template data_vector<tb_input_precision> operator-
    <tb_input_precision>(const data_vector<tb_input_precision>& v1, const data_vector<tb_input_precision>& v2);
template data_vector<tb_output_precision> operator-
    <tb_output_precision>(const data_vector<tb_output_precision>& v1, const data_vector<tb_output_precision>& v2);

template <class dtype>
data_vector<dtype> operator*(const data_vector<dtype>& v1, const data_vector<dtype>& v2) {
  if (v1.v.size() != v2.v.size()) {
    cerr << "Attempting to multiply two data vectors of different sizes!";
    cerr << "(" << v1.v.size() << "," << v2.v.size() << ")" << endl;
    exit(1);
  }

  data_vector<dtype> res = data_vector<dtype>(v1.v.size());
  for (unsigned int i = 0; i < v1.v.size(); i++) res.v[i] = v1.v[i] * v2.v[i];
  return res;
}
template data_vector<tb_input_precision> operator*
    <tb_input_precision>(const data_vector<tb_input_precision>& v1, const data_vector<tb_input_precision>& v2);
template data_vector<tb_output_precision> operator*
    <tb_output_precision>(const data_vector<tb_output_precision>& v1, const data_vector<tb_output_precision>& v2);

template <class dtype>
data_vector<dtype> max(const data_vector<dtype>& v1, const data_vector<dtype>& v2) {
  if (v1.v.size() != v2.v.size()) {
    cerr << "Attempting to max two data vectors of different sizes!";
    cerr << "(" << v1.v.size() << "," << v2.v.size() << ")" << endl;
    exit(1);
  }

  data_vector<dtype> res = data_vector<dtype>(v1.v.size());
  for (unsigned int i = 0; i < v1.v.size(); i++) res.v[i] = (v1.v[i] > v2.v[i]) ? v1.v[i] : v2.v[i];
  return res;
}
template data_vector<tb_input_precision> max<tb_input_precision>(const data_vector<tb_input_precision>& v1,
                                                                 const data_vector<tb_input_precision>& v2);
template data_vector<tb_output_precision> max<tb_output_precision>(const data_vector<tb_output_precision>& v1,
                                                                   const data_vector<tb_output_precision>& v2);

bool ParseNPUInst(std::string& inst_filename, std::vector<vliw_inst>& npu_program) {
  std::ifstream inst_file(inst_filename);
  if (!inst_file) return false;

  std::string line;
  unsigned int line_num = 0;
  unsigned int temp;
  vliw_inst npu_inst;

  while (getline(inst_file, line)) {
    std::stringstream line_stream(line);

    if (line_num == 0) {  // Read MVU Instruction
      line_stream >> temp;
      npu_inst.mvu_inst.op = temp;
      for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
        line_stream >> temp;
        npu_inst.mvu_inst.vrf_base[i] = temp;
      }
      line_stream >> temp;
      npu_inst.mvu_inst.vrf_size = temp;
      line_stream >> temp;
      npu_inst.mvu_inst.mrf_base = temp;
      line_stream >> temp;
      npu_inst.mvu_inst.mrf_size = temp;
      line_stream >> temp;
      npu_inst.mvu_inst.words_per_row = temp;
      line_stream >> temp;
      npu_inst.mvu_inst.tag = temp;
    } else if (line_num == 1) {  // Read eVRF Instruction
      line_stream >> temp;
      npu_inst.evrf_inst.op = temp;
      line_stream >> temp;
      npu_inst.evrf_inst.src_sel = temp;
      for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
        line_stream >> temp;
        npu_inst.evrf_inst.vrf_base[i] = temp;
      }
      line_stream >> temp;
      npu_inst.evrf_inst.vrf_size = temp;
      line_stream >> temp;
      npu_inst.evrf_inst.batch = temp;
      line_stream >> temp;
      npu_inst.evrf_inst.tag = temp;
    } else if (line_num == 2) {  // Read MFU0 Instruction
      line_stream >> temp;
      npu_inst.mfu0_inst.op = (temp << 6);
      line_stream >> temp;
      npu_inst.mfu0_inst.vrf_size = temp;
      line_stream >> temp;
      npu_inst.mfu0_inst.op += (temp << 4);
      line_stream >> temp;
      npu_inst.mfu0_inst.op += (temp << 1);
      for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
        line_stream >> temp;
        npu_inst.mfu0_inst.add_vrf_base[i] = temp;
      }
      line_stream >> temp;
      npu_inst.mfu0_inst.op += temp;
      for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
        line_stream >> temp;
        npu_inst.mfu0_inst.mul_vrf_base[i] = temp;
      }
      line_stream >> temp;
      npu_inst.mfu0_inst.batch = temp;
      line_stream >> temp;
      npu_inst.mfu0_inst.tag = temp;
    } else if (line_num == 3) {  // Read MFU1 Instruction
      line_stream >> temp;
      npu_inst.mfu1_inst.op = (temp << 6);
      line_stream >> temp;
      npu_inst.mfu1_inst.vrf_size = temp;
      line_stream >> temp;
      npu_inst.mfu1_inst.op += (temp << 4);
      line_stream >> temp;
      npu_inst.mfu1_inst.op += (temp << 1);
      for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
        line_stream >> temp;
        npu_inst.mfu1_inst.add_vrf_base[i] = temp;
      }
      line_stream >> temp;
      npu_inst.mfu1_inst.op += temp;
      for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
        line_stream >> temp;
        npu_inst.mfu1_inst.mul_vrf_base[i] = temp;
      }
      line_stream >> temp;
      npu_inst.mfu1_inst.batch = temp;
      line_stream >> temp;
      npu_inst.mfu1_inst.tag = temp;
    } else if (line_num == 4) {  // Read LD Instruction
      line_stream >> temp;
      npu_inst.ld_inst.op = temp;
      line_stream >> temp;
      npu_inst.ld_inst.src_sel = temp;
      line_stream >> temp;
      npu_inst.ld_inst.vrf_size = temp;

      bool wb_to_ch1 = false;
      line_stream >> temp;
      if (temp == 1) {
        line_stream >> temp;
        if (temp < TILES) {
          npu_inst.ld_inst.vrf0_id = temp + 1;
          npu_inst.ld_inst.vrf1_id = 0;
        } else {
          wb_to_ch1 = true;
          npu_inst.ld_inst.vrf0_id = 0;
          if (temp == TILES) {
            npu_inst.ld_inst.block1_id = 1;
            npu_inst.ld_inst.vrf1_id = 1;
          } else if (temp == TILES + 1) {
            npu_inst.ld_inst.block1_id = 2;
            npu_inst.ld_inst.vrf1_id = 1;
          } else if (temp == TILES + 2) {
            npu_inst.ld_inst.block1_id = 2;
            npu_inst.ld_inst.vrf1_id = 2;
          } else if (temp == TILES + 3) {
            npu_inst.ld_inst.block1_id = 3;
            npu_inst.ld_inst.vrf1_id = 1;
          } else if (temp == TILES + 4) {
            npu_inst.ld_inst.block1_id = 3;
            npu_inst.ld_inst.vrf1_id = 2;
          }
        }
      } else {
        line_stream >> temp;
        npu_inst.ld_inst.vrf0_id = 0;
      }
      if (temp < TILES) {
        for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
          line_stream >> temp;
          npu_inst.ld_inst.vrf0_base[i] = temp;
        }
      } else {
        for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
          line_stream >> temp;
          npu_inst.ld_inst.vrf0_base[i] = 0;
          npu_inst.ld_inst.vrf1_base[i] = temp;
        }
      }

      line_stream >> temp;
      if (temp == 1) {
        line_stream >> temp;
        if (temp == TILES + 1) {
          npu_inst.ld_inst.block1_id = 2;
          npu_inst.ld_inst.vrf1_id += 1;
        } else if (temp == TILES + 2) {
          npu_inst.ld_inst.block1_id = 2;
          npu_inst.ld_inst.vrf1_id += 2;
        } else if (temp == TILES + 3) {
          npu_inst.ld_inst.block1_id = 3;
          npu_inst.ld_inst.vrf1_id += 1;
        } else if (temp == TILES + 4) {
          npu_inst.ld_inst.block1_id = 3;
          npu_inst.ld_inst.vrf1_id += 2;
        }
      } else {
        line_stream >> temp;
        if (!wb_to_ch1) {
          npu_inst.ld_inst.block1_id = 0;
          npu_inst.ld_inst.vrf1_id = 0;
        }
      }
      if (!wb_to_ch1) {
        for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
          line_stream >> temp;
          npu_inst.ld_inst.vrf1_base[i] = temp;
        }
      } else {
        for (unsigned int i = 0; i < TB_NUM_DOTS; i++) {
          line_stream >> temp;
        }
      }

      line_stream >> temp;
      npu_inst.ld_inst.batch = temp;
      line_stream >> temp;
      npu_inst.ld_inst.output_result = temp;
    }

    if (line_num == 4) {
      npu_program.push_back(npu_inst);
      std::cout << "Instruction " << npu_program.size() - 1 << std::endl;
      std::cout << npu_inst << std::endl;
      std::cout << "--------------" << std::endl;
      line_num = 0;
    } else {
      line_num++;
    }
  }
  return true;
}

bool ParseNPUInputs(std::string& input_filename, std::vector<std::vector<tb_input_precision>>& npu_inputs) {
  std::ifstream inputs_file(input_filename);
  if (!inputs_file) return false;

  std::string line;
  while (std::getline(inputs_file, line)) {
    std::stringstream line_stream(line);
    std::vector<tb_input_precision> input_vector(LANES, 0);
    tb_input_precision value;
    int idx = 0;
    while (idx < LANES) {
      line_stream >> value;
      input_vector[idx] = value;
      idx++;
    }
    npu_inputs.push_back(input_vector);
  }
  return true;
}

bool ParseNPUOutputs(std::string& outputs_filename, std::vector<std::vector<tb_output_precision>>& npu_outputs) {
  std::ifstream outputs_file(outputs_filename);
  if (!outputs_file) return false;

  std::string line;
  while (std::getline(outputs_file, line)) {
    std::stringstream line_stream(line);
    std::vector<tb_output_precision> output_vector(LANES, 0);
    tb_output_precision value;
    int idx = 0;
    while (idx < LANES) {
      line_stream >> value;
      output_vector[idx] = value;
      idx++;
    }
    npu_outputs.push_back(output_vector);
  }
  return true;
}

void sc_trace(sc_trace_file* f, const data_vector<tb_output_precision>& dvector, const std::string& s) {
  for (unsigned int i = 0; i < dvector.v.size(); i++) sc_trace(f, dvector.v[i], s + "_v" + std::to_string(i));
}

void sc_trace(sc_trace_file* f, const data_vector<tb_input_precision>& dvector, const std::string& s) {
  for (unsigned int i = 0; i < dvector.v.size(); i++) sc_trace(f, dvector.v[i], s + "_v" + std::to_string(i));
}

template <typename dtype>
void init_vector<dtype>::init_sc_vector(sc_vector<dtype>& vector, unsigned int dim0) {
  vector.init(dim0);
}

template <typename dtype>
void init_vector<dtype>::init_sc_vector(sc_vector<std::vector<dtype>>& vector, unsigned int dim0, unsigned int dim1) {
  vector.init(dim0, [&](const char* nm, size_t) { (void) nm; return new std::vector<dtype>(dim1); });
}

template <typename dtype>
void init_vector<dtype>::init_sc_vector(sc_vector<sc_vector<dtype>>& vector, unsigned int dim0, unsigned int dim1) {
  vector.init(dim0, [&](const char* nm, size_t) { return new sc_vector<dtype>(nm, dim1); });
}

template <typename dtype>
void init_vector<dtype>::init_sc_vector(sc_vector<sc_vector<sc_vector<dtype>>>& vector, unsigned int dim0,
                                        unsigned int dim1, unsigned int dim2) {
  vector.init(dim0, [&](const char* nm, size_t) {
    auto temp = new sc_vector<sc_vector<dtype>>(nm);
    temp->init(dim1, [&](const char* nm, size_t) { return new sc_vector<dtype>(nm, dim2); });
    return temp;
  });
}

template <typename dtype>
void init_vector<dtype>::init_sc_vector(sc_vector<sc_vector<sc_vector<sc_vector<dtype>>>>& vector, unsigned int dim0,
                                        unsigned int dim1, unsigned int dim2, unsigned int dim3) {
  vector.init(dim0, [&](const char* nm, size_t) {
    auto temp0 = new sc_vector<sc_vector<sc_vector<dtype>>>(nm);
    temp0->init(dim1, [&](const char* nm, size_t) {
      auto temp1 = new sc_vector<sc_vector<dtype>>(nm);
      temp1->init(dim2, [&](const char* nm, size_t) { return new sc_vector<dtype>(nm, dim3); });
      return temp1;
    });
    return temp0;
  });
}

template class data_vector<tb_input_precision>;
template class data_vector<tb_output_precision>;

template class init_vector<sc_in<bool>>;
template class init_vector<sc_in<tb_input_precision>>;
template class init_vector<sc_in<tb_output_precision>>;
template class init_vector<sc_in<data_vector<tb_input_precision>>>;
template class init_vector<sc_in<data_vector<tb_output_precision>>>;
template class init_vector<sc_in<mvu_mop>>;
template class init_vector<sc_in<evrf_mop>>;
template class init_vector<sc_in<mfu_mop>>;
template class init_vector<sc_in<ld_mop>>;

template class init_vector<sc_out<bool>>;
template class init_vector<sc_out<tb_input_precision>>;
template class init_vector<sc_out<tb_output_precision>>;
template class init_vector<sc_out<data_vector<tb_input_precision>>>;
template class init_vector<sc_out<data_vector<tb_output_precision>>>;
template class init_vector<sc_out<mvu_mop>>;
template class init_vector<sc_out<evrf_mop>>;
template class init_vector<sc_out<mfu_mop>>;
template class init_vector<sc_out<ld_mop>>;

template class init_vector<sc_signal<bool>>;
template class init_vector<sc_signal<unsigned int>>;
template class init_vector<sc_signal<tb_input_precision>>;
template class init_vector<sc_signal<tb_output_precision>>;
template class init_vector<sc_signal<data_vector<tb_input_precision>>>;
template class init_vector<sc_signal<data_vector<tb_output_precision>>>;
template class init_vector<sc_signal<mvu_mop>>;
template class init_vector<sc_signal<evrf_mop>>;
template class init_vector<sc_signal<mfu_mop>>;
template class init_vector<sc_signal<ld_mop>>;

template class init_vector<axis_master_port>;
template class init_vector<axis_slave_port>;
template class init_vector<axis_signal>;