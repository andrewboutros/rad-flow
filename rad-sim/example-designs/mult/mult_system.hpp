#pragma once

#include <mult_driver.hpp>
#include <mult_top.hpp>
#include <chrono>
#include <vector>

class mult_system : public sc_module {
private:
  sc_signal<sc_bv<DATAW>> client_tdata_sig;
  sc_signal<bool> client_tlast_sig;
  sc_signal<bool> client_valid_sig;
  sc_signal<bool> client_ready_sig;
  sc_signal<sc_bv<DATAW>> response_sig;
  sc_signal<bool> response_valid_sig;

public:
  sc_signal<bool> rst_sig;
  sc_clock *sysclk;
  mult_driver *driver_inst;
  mult_top *dut_inst;
  //AKB added:
  //sc_signal<bool> portal_in_sig;
  //sc_signal<bool> portal_out_sig;
  //sc_in<bool> portal_in;
  //sc_out<bool> portal_out;

  mult_system(const sc_module_name &name,
                           sc_clock *driver_clk_sig, RADSimDesignContext* radsim_design); //AKB added last arg
  ~mult_system();
};