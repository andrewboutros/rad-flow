#pragma once

#include <systemc.h>

#include <axis_interface.hpp>
#include <fstream>
#include <instructions.hpp>
#include <iostream>
#include <params.hpp>
#include <sstream>

int GetNPUSimCycle(double period);
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
  void load_from_bv(sc_bv<FEEDFORWARD_DATA_WIDTH>& bv){};
  void load_from_bv(sc_bv<WIDE_WRITEBACK_BV_WIDTH>& bv){};
  void load_from_bv(sc_bv<NARROW_WRITEBACK_BV_WIDTH>& bv){};
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

struct debug_t {};
constexpr auto debug = debug_t{};
struct trace_t {};
constexpr auto sim_trace = trace_t{};
struct info_t {};
constexpr auto info = info_t{};
struct critical_t {};
constexpr auto critical = critical_t{};
struct warning_t {};
constexpr auto warning = warning_t{};
struct error_t {};
constexpr auto error = error_t{};

class sim_log {
 private:
  unsigned int verbosity;
  std::ofstream log_file;

 public:
  sim_log(unsigned int verbosity_level, std::string log_filename);
  ~sim_log();
  void log(debug_t, std::string msg, sc_module_name module = "", bool log_to_file = true);
  void log(trace_t, std::string msg, sc_module_name module, bool log_to_file = true);
  void log(info_t, std::string msg, sc_module_name module = "", bool log_to_file = true);
  void log(critical_t, std::string msg, sc_module_name module = "", bool log_to_file = true);
  void log(warning_t, std::string msg, sc_module_name module = "", bool log_to_file = true);
  void log(error_t, std::string msg, sc_module_name module = "", bool log_to_file = true);
};

class sim_trace_probe {
 private:
  std::vector<std::vector<std::vector<unsigned int>>> trace_cycles;
  unsigned int num_traces_monitored;
  unsigned int num_modules_monitored;
  std::string trace_filename;

 public:
  sim_trace_probe(std::string filename, unsigned int num_traces, unsigned int num_modules);
  ~sim_trace_probe();
  void record_event(unsigned int module_id, unsigned int trace_id);
  void dump_traces();
};

extern sim_log npu_log;
extern sim_trace_probe npu_trace_probe;