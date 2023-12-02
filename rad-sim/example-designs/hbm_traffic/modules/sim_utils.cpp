#include <axis_interface.hpp>
#include <sim_utils.hpp>

template <class dtype> data_vector<dtype>::data_vector() {}

template <class dtype> data_vector<dtype>::data_vector(unsigned int size) {
  v.resize(size, 0);
}

template <class dtype>
data_vector<dtype>::data_vector(sc_vector<sc_in<dtype>> &iport) {
  v.resize(iport.size());
  for (unsigned int i = 0; i < iport.size(); i++)
    v[i] = iport[i].read();
}

template <class dtype>
data_vector<dtype>::data_vector(sc_vector<sc_signal<dtype>> &sig_vec) {
  v.resize(sig_vec.size());
  for (unsigned int i = 0; i < sig_vec.size(); i++)
    v[i] = sig_vec[i].read();
}

template <class dtype>
data_vector<dtype>::data_vector(std::vector<dtype> &vec) {
  v.resize(vec.size());
  for (unsigned int i = 0; i < vec.size(); i++)
    v[i] = vec[i];
}

template <class dtype> data_vector<dtype>::data_vector(std::vector<int> &vec) {
  v.resize(vec.size());
  for (unsigned int i = 0; i < vec.size(); i++)
    v[i] = vec[i];
}

// template <class dtype> data_vector<dtype>::data_vector(std::vector<bool> &vec) {
//   v.resize(vec.size());
//   for (unsigned int i = 0; i < vec.size(); i++)
//     v[i] = vec[i];
// }

template <class dtype>
bool data_vector<dtype>::operator==(const data_vector<dtype> &rhs) {
  if (v.size() != rhs.v.size())
    return false;
  bool is_equal = true;
  for (unsigned int i = 0; i < v.size(); i++)
    is_equal &= (v[i] == rhs.v[i]);
  return is_equal;
}

template <class dtype>
bool data_vector<dtype>::operator==(const std::vector<dtype> &rhs) {
  if (v.size() != rhs.size())
    return false;
  bool is_equal = true;
  for (unsigned int i = 0; i < v.size(); i++)
    is_equal &= (v[i] == rhs[i]);
  return is_equal;
}

template <class dtype> dtype &data_vector<dtype>::operator[](unsigned int idx) {
  return v[idx];
}

template <class dtype> unsigned int data_vector<dtype>::size() {
  return v.size();
}

template <class dtype> void data_vector<dtype>::resize(unsigned int size) {
  return v.resize(size);
}

template <class dtype>
ostream &operator<<(ostream &o, const data_vector<dtype> &dvector) {
  for (unsigned int i = 0; i < dvector.v.size(); i++)
    o << dvector.v[i] << " ";
  return o;
}
template ostream &
operator<< <unsigned int>(ostream &o, const data_vector<unsigned int> &dvector);
template ostream &operator<< <uint64_t>(ostream &o,
                                        const data_vector<uint64_t> &dvector);
template ostream &operator<< <int16_t>(ostream &o,
                                       const data_vector<int16_t> &dvector);
template ostream &
operator<< <sc_int<32>>(ostream &o, const data_vector<sc_int<32>> &dvector);
template ostream &operator<< <sc_int<5>>(ostream &o,
                                         const data_vector<sc_int<5>> &dvector);
template ostream &
operator<< <sc_uint<5>>(ostream &o, const data_vector<sc_uint<5>> &dvector);

template <class dtype>
data_vector<dtype> operator+(const data_vector<dtype> &v1,
                             const data_vector<dtype> &v2) {
  assert(v1.v.size() == v2.v.size());
  if (v1.v.size() != v2.v.size()) {
    cerr << "Attempting to add two data vectors of different sizes! ";
    cerr << "(" << v1.v.size() << "," << v2.v.size() << ")" << endl;
    exit(1);
  }

  data_vector<dtype> res = data_vector<dtype>(v1.v.size());
  for (unsigned int i = 0; i < v1.v.size(); i++)
    res.v[i] = v1.v[i] + v2.v[i];
  return res;
}
template data_vector<unsigned int> operator+
    <unsigned int>(const data_vector<unsigned int> &v1,
                   const data_vector<unsigned int> &v2);
template data_vector<uint64_t> operator+
    <uint64_t>(const data_vector<uint64_t> &v1,
               const data_vector<uint64_t> &v2);
template data_vector<int16_t> operator+
    <int16_t>(const data_vector<int16_t> &v1, const data_vector<int16_t> &v2);
template data_vector<sc_int<32>> operator+
    <sc_int<32>>(const data_vector<sc_int<32>> &v1,
                 const data_vector<sc_int<32>> &v2);
template data_vector<sc_int<5>> operator+
    <sc_int<5>>(const data_vector<sc_int<5>> &v1,
                const data_vector<sc_int<5>> &v2);
template data_vector<sc_uint<5>> operator+
    <sc_uint<5>>(const data_vector<sc_uint<5>> &v1,
                 const data_vector<sc_uint<5>> &v2);

template <class dtype>
data_vector<dtype> operator-(const data_vector<dtype> &v1,
                             const data_vector<dtype> &v2) {
  if (v1.v.size() != v2.v.size()) {
    cerr << "Attempting to subtract two data vectors of different sizes!";
    cerr << "(" << v1.v.size() << "," << v2.v.size() << ")" << endl;
    exit(1);
  }

  data_vector<dtype> res = data_vector<dtype>(v1.v.size());
  for (unsigned int i = 0; i < v1.v.size(); i++)
    res.v[i] = v1.v[i] - v2.v[i];
  return res;
}
template data_vector<unsigned int> operator-
    <unsigned int>(const data_vector<unsigned int> &v1,
                   const data_vector<unsigned int> &v2);
template data_vector<uint64_t> operator-
    <uint64_t>(const data_vector<uint64_t> &v1,
               const data_vector<uint64_t> &v2);
template data_vector<int16_t> operator-
    <int16_t>(const data_vector<int16_t> &v1, const data_vector<int16_t> &v2);
template data_vector<sc_int<32>> operator-
    <sc_int<32>>(const data_vector<sc_int<32>> &v1,
                 const data_vector<sc_int<32>> &v2);
template data_vector<sc_int<5>> operator-
    <sc_int<5>>(const data_vector<sc_int<5>> &v1,
                const data_vector<sc_int<5>> &v2);
template data_vector<sc_uint<5>> operator-
    <sc_uint<5>>(const data_vector<sc_uint<5>> &v1,
                 const data_vector<sc_uint<5>> &v2);

template <class dtype>
data_vector<dtype> operator*(const data_vector<dtype> &v1,
                             const data_vector<dtype> &v2) {
  if (v1.v.size() != v2.v.size()) {
    cerr << "Attempting to multiply two data vectors of different sizes!";
    cerr << "(" << v1.v.size() << "," << v2.v.size() << ")" << endl;
    exit(1);
  }

  data_vector<dtype> res = data_vector<dtype>(v1.v.size());
  for (unsigned int i = 0; i < v1.v.size(); i++)
    res.v[i] = v1.v[i] * v2.v[i];
  return res;
}
template data_vector<unsigned int> operator*
    <unsigned int>(const data_vector<unsigned int> &v1,
                   const data_vector<unsigned int> &v2);
template data_vector<uint64_t> operator*
    <uint64_t>(const data_vector<uint64_t> &v1,
               const data_vector<uint64_t> &v2);
template data_vector<int16_t> operator*
    <int16_t>(const data_vector<int16_t> &v1, const data_vector<int16_t> &v2);
template data_vector<sc_int<32>> operator*
    <sc_int<32>>(const data_vector<sc_int<32>> &v1,
                 const data_vector<sc_int<32>> &v2);
template data_vector<sc_int<5>> operator*
    <sc_int<5>>(const data_vector<sc_int<5>> &v1,
                const data_vector<sc_int<5>> &v2);
template data_vector<sc_uint<5>> operator*
    <sc_uint<5>>(const data_vector<sc_uint<5>> &v1,
                 const data_vector<sc_uint<5>> &v2);

template <class dtype>
data_vector<dtype> max(const data_vector<dtype> &v1,
                       const data_vector<dtype> &v2) {
  if (v1.v.size() != v2.v.size()) {
    cerr << "Attempting to max two data vectors of different sizes!";
    cerr << "(" << v1.v.size() << "," << v2.v.size() << ")" << endl;
    exit(1);
  }

  data_vector<dtype> res = data_vector<dtype>(v1.v.size());
  for (unsigned int i = 0; i < v1.v.size(); i++)
    res.v[i] = (v1.v[i] > v2.v[i]) ? v1.v[i] : v2.v[i];
  return res;
}
template data_vector<unsigned int>
max<unsigned int>(const data_vector<unsigned int> &v1,
                  const data_vector<unsigned int> &v2);
template data_vector<uint64_t> max<uint64_t>(const data_vector<uint64_t> &v1,
                                             const data_vector<uint64_t> &v2);
template data_vector<int16_t> max<int16_t>(const data_vector<int16_t> &v1,
                                           const data_vector<int16_t> &v2);
template data_vector<sc_int<32>>
max<sc_int<32>>(const data_vector<sc_int<32>> &v1,
                const data_vector<sc_int<32>> &v2);
template data_vector<sc_int<5>>
max<sc_int<5>>(const data_vector<sc_int<5>> &v1,
               const data_vector<sc_int<5>> &v2);
template data_vector<sc_uint<5>>
max<sc_uint<5>>(const data_vector<sc_uint<5>> &v1,
                const data_vector<sc_uint<5>> &v2);

void sc_trace(sc_trace_file *f, const data_vector<unsigned int> &dvector,
              const std::string &s) {
  for (unsigned int i = 0; i < dvector.v.size(); i++)
    sc_trace(f, dvector.v[i], s + "_v" + std::to_string(i));
}
void sc_trace(sc_trace_file *f, const data_vector<uint64_t> &dvector,
              const std::string &s) {
  for (unsigned int i = 0; i < dvector.v.size(); i++)
    sc_trace(f, dvector.v[i], s + "_v" + std::to_string(i));
}
void sc_trace(sc_trace_file *f, const data_vector<int16_t> &dvector,
              const std::string &s) {
  for (unsigned int i = 0; i < dvector.v.size(); i++)
    sc_trace(f, dvector.v[i], s + "_v" + std::to_string(i));
}
void sc_trace(sc_trace_file *f, const data_vector<sc_int<32>> &dvector,
              const std::string &s) {
  for (unsigned int i = 0; i < dvector.v.size(); i++)
    sc_trace(f, dvector.v[i], s + "_v" + std::to_string(i));
}
void sc_trace(sc_trace_file *f, const data_vector<sc_int<5>> &dvector,
              const std::string &s) {
  for (unsigned int i = 0; i < dvector.v.size(); i++)
    sc_trace(f, dvector.v[i], s + "_v" + std::to_string(i));
}
void sc_trace(sc_trace_file *f, const data_vector<sc_uint<5>> &dvector,
              const std::string &s) {
  for (unsigned int i = 0; i < dvector.v.size(); i++)
    sc_trace(f, dvector.v[i], s + "_v" + std::to_string(i));
}

template <typename dtype>
void init_vector<dtype>::init_sc_vector(sc_vector<dtype> &vector,
                                        unsigned int dim0) {
  vector.init(dim0);
}

template class data_vector<unsigned int>;
template class data_vector<uint64_t>;
template class data_vector<int16_t>;

template class init_vector<sc_in<data_vector<unsigned int>>>;
template class init_vector<sc_out<data_vector<unsigned int>>>;
template class init_vector<sc_signal<unsigned int>>;

template class init_vector<sc_in<data_vector<uint64_t>>>;
template class init_vector<sc_out<data_vector<uint64_t>>>;
template class init_vector<sc_signal<uint64_t>>;

template class init_vector<sc_in<data_vector<int16_t>>>;
template class init_vector<sc_out<data_vector<int16_t>>>;
template class init_vector<sc_signal<int16_t>>;

template class data_vector<sc_int<32>>;
template class data_vector<sc_int<5>>;
template class data_vector<sc_uint<5>>;

template class init_vector<sc_in<bool>>;
template class init_vector<sc_out<bool>>;
template class init_vector<sc_signal<bool>>;
template class init_vector<sc_in<data_vector<sc_int<32>>>>;
template class init_vector<sc_out<data_vector<sc_int<32>>>>;
template class init_vector<sc_signal<data_vector<sc_int<32>>>>;




template <typename T>
void axis_bv_to_data_vector(
    sc_bv<AXIS_MAX_DATAW> &bitvector, 
    data_vector<T> &datavector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * bitwidth;
    end_idx = (e + 1) * bitwidth;
    datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
  }
}

template <typename T>
void aximm_bv_to_data_vector(
    sc_bv<AXI4_MAX_DATAW> &bitvector, 
    data_vector<T> &datavector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * bitwidth;
    end_idx = (e + 1) * bitwidth;
    datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
  }
}

void aximm_bv_to_data_vector(
    sc_bv<AXI4_MAX_DATAW> &bitvector, 
    data_vector<int16_t> &datavector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * bitwidth;
    end_idx = (e + 1) * bitwidth;
    datavector[e] = bitvector.range(end_idx - 1, start_idx).to_int();
  }
}

void aximm_512_bv_to_64_data_vector(
    sc_bv<AXI4_MAX_DATAW> &bitvector, 
    data_vector<uint64_t> &datavector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {

  unsigned int start_idx, end_idx;
  for (unsigned int e = 0; e < num_elements; e++) {
    start_idx = e * bitwidth;
    end_idx = (e + 1) * bitwidth;
    datavector[e] = bitvector.range(end_idx - 1, start_idx).to_uint64();
  }
}


template <typename T>
void data_vector_to_bv_axis( 
    data_vector<T> &datavector, 
    sc_bv<AXIS_MAX_DATAW> &bitvector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {
    unsigned int start_idx, end_idx;
    for (unsigned int e = 0; e < num_elements; e++) {
        start_idx = e * bitwidth;
        end_idx = (e + 1) * bitwidth;
        bitvector.range(end_idx - 1, start_idx) = datavector[e];
    }
}

template <typename T>
void data_vector_to_bv_aximm( 
    data_vector<T> &datavector, 
    sc_bv<AXI4_MAX_DATAW> &bitvector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {
    unsigned int start_idx, end_idx;
    for (unsigned int e = 0; e < num_elements; e++) {
        start_idx = e * bitwidth;
        end_idx = (e + 1) * bitwidth;
        bitvector.range(end_idx - 1, start_idx) = datavector[e];
    }
}

void data_vector_to_bv_axis( 
    data_vector<int16_t> &datavector, 
    sc_bv<AXIS_MAX_DATAW> &bitvector, 
    unsigned int bitwidth, 
    unsigned int num_elements) {
    unsigned int start_idx, end_idx;
    for (unsigned int e = 0; e < num_elements; e++) {
        start_idx = e * bitwidth;
        end_idx = (e + 1) * bitwidth;
        bitvector.range(end_idx - 1, start_idx) = datavector[e];
    }
}