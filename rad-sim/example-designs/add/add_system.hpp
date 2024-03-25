#pragma once

#include <add_driver.hpp>
#include <add_top.hpp>
#include <chrono>
#include <vector>
#include <design_system.hpp>

class add_system : public design_system {
private:
  sc_signal<sc_bv<DATAW>> client_tdata_sig;
  sc_signal<bool> client_tlast_sig;
  sc_signal<bool> client_valid_sig;
  sc_signal<bool> client_ready_sig;
  sc_signal<sc_bv<DATAW>> response_sig;
  sc_signal<bool> response_valid_sig;
  sc_signal<bool> portal_recvd_sig;

public:
  sc_signal<bool> rst_sig;
  sc_clock *sysclk;
  add_driver *driver_inst;
  add_top *dut_inst;

  add_system(const sc_module_name &name,
                           sc_clock *driver_clk_sig, RADSimDesignContext* radsim_design);
  ~add_system();
};