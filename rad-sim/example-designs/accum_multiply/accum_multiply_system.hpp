#pragma once

#include <aximm_hello_world_driver.hpp>
#include <aximm_hello_world_top.hpp>
#include <chrono>
#include <vector>

class aximm_hello_world_system : public sc_module {
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
  aximm_hello_world_driver *driver_inst;
  aximm_hello_world_top *dut_inst;

  aximm_hello_world_system(const sc_module_name &name,
                           sc_clock *driver_clk_sig);
  ~aximm_hello_world_system();
};