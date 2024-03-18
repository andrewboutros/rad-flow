#pragma once

#include <one_to_one_driver.hpp>
#include <one_to_one_top.hpp>
#include <chrono>
#include <vector>

class one_to_one_system : public sc_module {
private:
  sc_signal<uint64_t> req_addr_sig;
  sc_signal<bool> req_type_sig;
  sc_signal<sc_bv<DATAW>> req_wdata_sig;
  sc_signal<bool> req_valid_sig;
  sc_signal<bool> req_ready_sig;
  sc_signal<unsigned int> received_responses_sig;

public:
  sc_signal<bool> rst_sig;
  sc_clock *sysclk;
  one_to_one_driver *driver_inst;
  one_to_one_top *top_inst;

  one_to_one_system(const sc_module_name &name,
                           sc_clock *driver_clk_sig);
  ~one_to_one_system();
};