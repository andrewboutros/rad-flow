#pragma once

#include <two_to_two_driver.hpp>
#include <two_to_two_top.hpp>
#include <chrono>
#include <vector>

class two_to_two_system : public sc_module {
private:
  sc_signal<uint64_t> req_addr_sig;
  sc_signal<bool> req_type_sig;
  sc_signal<sc_bv<DATAW>> req_wdata_sig;
  sc_signal<bool> req_valid_sig;
  sc_signal<bool> req_ready_sig;
  sc_signal<unsigned int> received_responses_sig;

  sc_signal<uint64_t> req_addr_sig_2;
  sc_signal<bool> req_type_sig_2;
  sc_signal<sc_bv<DATAW>> req_wdata_sig_2;
  sc_signal<bool> req_valid_sig_2;
  sc_signal<bool> req_ready_sig_2;
  sc_signal<unsigned int> received_responses_sig_2;

public:
  sc_signal<bool> rst_sig;
  sc_signal<bool> rst_sig_2;
  sc_clock *sysclk;
  two_to_two_driver *driver_inst;
  two_to_two_top *top_inst;

  two_to_two_system(const sc_module_name &name,
                           sc_clock *driver_clk_sig);
  ~two_to_two_system();
};