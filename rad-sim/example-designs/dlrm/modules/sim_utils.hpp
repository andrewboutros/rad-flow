#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <systemc.h>

template <class dtype> class data_vector;
template <class T>
data_vector<T> max(const data_vector<T> &v1, const data_vector<T> &v2);
template <class T>
ostream &operator<<(ostream &o, const data_vector<T> &dvector);
template <class T>
data_vector<T> operator+(const data_vector<T> &v1, const data_vector<T> &v2);
template <class T>
data_vector<T> operator-(const data_vector<T> &v1, const data_vector<T> &v2);
template <class T>
data_vector<T> operator*(const data_vector<T> &v1, const data_vector<T> &v2);

template <class dtype> class data_vector : public std::error_code {
public:
  std::vector<dtype> v;

  data_vector();
  data_vector(unsigned int size);
  data_vector(sc_vector<sc_in<dtype>> &iport);
  data_vector(std::vector<dtype> &vec);
  data_vector(std::vector<int> &vec);
  bool operator==(const data_vector<dtype> &rhs);
  bool operator==(const std::vector<dtype> &rhs);
  dtype &operator[](unsigned int idx);
  unsigned int size();
  void resize(unsigned int size);
  friend ostream &operator<< <dtype>(ostream &o,
                                     const data_vector<dtype> &dvector);
  friend data_vector operator+
      <dtype>(const data_vector<dtype> &v1, const data_vector<dtype> &v2);
  friend data_vector operator-
      <dtype>(const data_vector<dtype> &v1, const data_vector<dtype> &v2);
  friend data_vector operator*
      <dtype>(const data_vector<dtype> &v1, const data_vector<dtype> &v2);
  friend data_vector max<dtype>(const data_vector<dtype> &v1,
                                const data_vector<dtype> &v2);
};
void sc_trace(sc_trace_file *f, const data_vector<unsigned int> &dvector,
              const std::string &s);
void sc_trace(sc_trace_file *f, const data_vector<uint64_t> &dvector,
              const std::string &s);
void sc_trace(sc_trace_file *f, const data_vector<int16_t> &dvector,
              const std::string &s);
void sc_trace(sc_trace_file *f, const data_vector<sc_int<32>> &dvector,
              const std::string &s);
void sc_trace(sc_trace_file *f, const data_vector<sc_int<5>> &dvector,
              const std::string &s);
void sc_trace(sc_trace_file *f, const data_vector<sc_uint<5>> &dvector,
              const std::string &s);

template <typename dtype> struct init_vector {
  static void init_sc_vector(sc_vector<dtype> &vector, unsigned int dim0);
};

template <> struct std::iterator_traits<unsigned int> {
  typedef int difference_type;
  typedef int value_type;
  typedef int *pointer;
  typedef int &reference;
  typedef std::forward_iterator_tag iterator_category;
};

template <> struct std::iterator_traits<uint64_t> {
  typedef int difference_type;
  typedef int value_type;
  typedef int *pointer;
  typedef int &reference;
  typedef std::forward_iterator_tag iterator_category;
};

template <> struct std::iterator_traits<int16_t> {
  typedef int difference_type;
  typedef int value_type;
  typedef int *pointer;
  typedef int &reference;
  typedef std::forward_iterator_tag iterator_category;
};

template <> struct std::iterator_traits<sc_int<32>> {
  typedef int difference_type;
  typedef int value_type;
  typedef int *pointer;
  typedef int &reference;
  typedef std::forward_iterator_tag iterator_category;
};

template <> struct std::iterator_traits<sc_int<5>> {
  typedef int difference_type;
  typedef int value_type;
  typedef int *pointer;
  typedef int &reference;
  typedef std::forward_iterator_tag iterator_category;
};

template <> struct std::iterator_traits<sc_uint<5>> {
  typedef int difference_type;
  typedef int value_type;
  typedef int *pointer;
  typedef int &reference;
  typedef std::forward_iterator_tag iterator_category;
};