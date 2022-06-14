#pragma once

#include <systemc.h>

#include <fstream>
#include <instructions.hpp>
#include <iostream>
#include <params.hpp>
#include <sstream>

bool ParseNPUInst(std::string& inst_filename, std::vector<vliw_inst>& npu_program);
bool ParseNPUInputs(std::string& input_filename, std::vector<std::vector<tb_input_precision>>& npu_inputs);
bool ParseNPUOutputs(std::string& output_filename, std::vector<std::vector<tb_output_precision>>& npu_outputs);

template <class dtype>
class data_vector;
template <class T>
data_vector<T> max(const data_vector<T>& v1, const data_vector<T>& v2);
template <class T>
ostream& operator<<(ostream& o, const data_vector<T>& dvector);
template <class T>
data_vector<T> operator+(const data_vector<T>& v1, const data_vector<T>& v2);
template <class T>
data_vector<T> operator-(const data_vector<T>& v1, const data_vector<T>& v2);
template <class T>
data_vector<T> operator*(const data_vector<T>& v1, const data_vector<T>& v2);

template <class dtype>
class data_vector : public std::error_code {
 public:
  std::vector<dtype> v;

  data_vector();
  data_vector(unsigned int size);
  data_vector(sc_vector<sc_in<dtype>>& iport);
  data_vector(std::vector<dtype>& vec);
  bool operator==(const data_vector<dtype>& rhs);
  bool operator==(const std::vector<dtype>& rhs);
  dtype& operator[](unsigned int idx);
  unsigned int size();
  void data_vector_truncate(const data_vector<tb_output_precision>& v1);
  friend ostream& operator<<<dtype>(ostream& o, const data_vector<dtype>& dvector);
  friend data_vector operator+<dtype>(const data_vector<dtype>& v1, const data_vector<dtype>& v2);
  friend data_vector operator-<dtype>(const data_vector<dtype>& v1, const data_vector<dtype>& v2);
  friend data_vector operator*<dtype>(const data_vector<dtype>& v1, const data_vector<dtype>& v2);
  friend data_vector max<dtype>(const data_vector<dtype>& v1, const data_vector<dtype>& v2);

  sc_bv<1> convert_to_bv() {
    sc_bv<1> temp = 0;
    return temp;
  };
  void load_from_bv(sc_bv<FEEDFORWARD_DATA_WIDTH>& bv){ (void) bv; };
  void load_from_bv(sc_bv<WIDE_WRITEBACK_BV_WIDTH>& bv){ (void) bv; };
  void load_from_bv(sc_bv<NARROW_WRITEBACK_BV_WIDTH>& bv){ (void) bv; };
};
void sc_trace(sc_trace_file* f, const data_vector<tb_output_precision>& dvector, const std::string& s);
void sc_trace(sc_trace_file* f, const data_vector<tb_input_precision>& dvector, const std::string& s);

template <typename dtype>
struct init_vector {
  static void init_sc_vector(sc_vector<dtype>& vector, unsigned int dim0);
  static void init_sc_vector(sc_vector<std::vector<dtype>>& vector, unsigned int dim0, unsigned int dim1);
  static void init_sc_vector(sc_vector<sc_vector<dtype>>& vector, unsigned int dim0, unsigned int dim1);
  static void init_sc_vector(sc_vector<sc_vector<sc_vector<dtype>>>& vector, unsigned int dim0, unsigned int dim1,
                             unsigned int dim2);
  static void init_sc_vector(sc_vector<sc_vector<sc_vector<sc_vector<dtype>>>>& vector, unsigned int dim0,
                             unsigned int dim1, unsigned int dim2, unsigned int dim3);
};

template<> struct std::iterator_traits<sc_int<8>> {
    typedef int difference_type;
    typedef int value_type;
    typedef int* pointer;
    typedef int& reference;
    typedef std::forward_iterator_tag iterator_category;
};

template<> struct std::iterator_traits<sc_int<32>> {
    typedef int difference_type;
    typedef int value_type;
    typedef int* pointer;
    typedef int& reference;
    typedef std::forward_iterator_tag iterator_category;
};