#pragma once

#include <rtl_aximm_driver.hpp>
#include <rtl_aximm_top.hpp>
#include <chrono>
#include <vector>

class rtl_aximm_system : public sc_module {
private:
  // Client Interface
  sc_signal<bool> start_sig;
	sc_signal<bool> start_rdy;
	sc_signal<bool> output_valid;
	sc_signal<bool> output_result;
	sc_signal<bool> output_rdy;

public:
  sc_signal<bool> rst_sig;
  sc_clock *sysclk;
  rtl_aximm_driver *driver_inst;
  rtl_aximm_top *dut_inst;

  rtl_aximm_system(const sc_module_name &name,
                           sc_clock *driver_clk_sig);
  ~rtl_aximm_system();
};