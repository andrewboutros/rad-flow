#pragma once

#include <mult_driver.hpp>
#include <mult_top.hpp>
#include <chrono>
#include <vector>
#include <design_system.hpp>

class mult_system : public RADSimDesignSystem {
private:
  sc_signal<sc_bv<DATAW>> client_tdata_sig;
  sc_signal<bool> client_tlast_sig;
  sc_signal<bool> client_valid_sig;
  sc_signal<bool> client_ready_sig;
  sc_signal<sc_bv<DATAW>> response_sig;
  sc_signal<bool> response_valid_sig;
  sc_signal<bool> mult_inter_rad_recvd_sig;

public:
  sc_signal<bool> rst_sig;
  sc_clock *sysclk;
  mult_driver *driver_inst;
  mult_top *dut_inst;

  mult_system(const sc_module_name &name,
                           sc_clock *driver_clk_sig, RADSimDesignContext* radsim_design);
  ~mult_system();
};