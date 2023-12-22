#include <rtl_aximm_driver.hpp>

rtl_aximm_driver::rtl_aximm_driver(const sc_module_name &name)
    : sc_module(name) {

  // Parse design configuration (number of layers & number of MVM per layer)
  std::string design_root_dir =
      radsim_config.GetStringKnob("radsim_user_design_root_dir");

  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

rtl_aximm_driver::~rtl_aximm_driver() {}

void rtl_aximm_driver::source() {
  // Reset
  rst.write(true);
  start_sig.write(false);
  wait();
  rst.write(false);
  wait();
  while (true) {
    if (start_rdy.read()) {
      start_sig.write(1);
    }

    wait();
  }
  
  wait();
}

void rtl_aximm_driver::sink() {
  std::string dst_port_name = "server_inst.aximm_server";
  uint64_t dst_addr = radsim_design.GetPortBaseAddress(dst_port_name);
  std::cout << dst_addr << std::endl;
  while(true) {
    if (output_valid) {
      if (output_result) {
        std::cout << "SUCCESS - R Transaction matches W Transaction!" << std::endl;
      } else {
        std::cout << "FAILURE - R Transaction does not match W Transaction!" << std::endl;
        radsim_design.ReportDesignFailure();
      }
      break;
    }
    wait();
  }
  sc_stop();
}