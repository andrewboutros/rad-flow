#pragma once

#include <sim_utils.hpp>
#include <sstream>
#include <systemc.h>

#define RF_RD_LATENCY 2
#define RF_WR_LATENCY 2

template <class dtype> class register_file : public sc_module {
private:
  unsigned int register_file_id;
  unsigned int num_elements_per_word;
  std::vector<std::vector<dtype>> mem;

  sc_vector<sc_signal<data_vector<dtype>>> wdata_pipeline;
  sc_vector<sc_signal<uint32_t>> waddr_pipeline;
  sc_vector<sc_signal<uint32_t>> raddr_pipeline;
  sc_vector<sc_signal<bool>> wen_pipeline;

public:
  // Register file inputs
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_in<uint32_t> raddr;
  sc_in<data_vector<dtype>> wdata;
  sc_in<uint32_t> waddr;
  sc_in<bool> wen;
  sc_in<bool> clk_en;

  // Register file outputs
  sc_out<data_vector<dtype>> rdata;

  // DPE constructor and destructor
  register_file(const sc_module_name &name, unsigned int id, unsigned int depth,
                unsigned int width, std::string &init_file);
  ~register_file();

  void Tick();
  void Assign();

  SC_HAS_PROCESS(register_file);
};

template <class dtype>
bool parse_register_file_contents_from_file(
    std::vector<std::vector<dtype>> &mem, std::string &init_file,
    unsigned int width, unsigned int depth) {

  std::ifstream rf_content(init_file);

  if (!rf_content)
    return false;

  std::string line;
  uint32_t addr = 0;
  while (std::getline(rf_content, line) && (addr < depth)) {
    std::stringstream line_stream(line);
    std::vector<dtype> rf_word(width, 0);
    dtype value;
    unsigned int idx = 0;
    while (idx < width) {
      line_stream >> value;
      rf_word[idx] = value;
      idx++;
    }
    mem[addr] = rf_word;
    addr++;
  }
  return true;
}