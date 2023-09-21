#pragma once

#include <rtl_add_driver.hpp>
#include <rtl_add_top.hpp>
#include <chrono>
#include <vector>

class rtl_add_system : public sc_module {
private:
  sc_signal<sc_bv<DATAW>> client_tdata_sig;
  sc_signal<bool> client_tlast_sig;
  sc_signal<bool> client_valid_sig;
  sc_signal<bool> client_ready_sig;

public:
  sc_signal<bool> rst_sig;
  sc_clock *sysclk;
  rtl_add_driver *driver_inst;
  rtl_add_top *dut_inst;

  rtl_add_system(const sc_module_name &name,
                           sc_clock *driver_clk_sig);
  ~rtl_add_system();
};