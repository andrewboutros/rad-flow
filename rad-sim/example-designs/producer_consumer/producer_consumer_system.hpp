#pragma once

#include <producer_consumer_top.hpp>
#include <chrono>
#include <vector>
#include <design_system.hpp>

class producer_consumer_system : public RADSimDesignSystem {
public:
  sc_signal<bool> rst_sig;
  sc_clock *sysclk;
  producer_consumer_top *dut_inst;

  producer_consumer_system(const sc_module_name &name, sc_clock *driver_clk_sig, RADSimDesignContext* radsim_design);
  ~producer_consumer_system();
};

