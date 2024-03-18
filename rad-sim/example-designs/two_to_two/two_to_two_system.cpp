#include <two_to_two_system.hpp>

two_to_two_system::two_to_two_system(const sc_module_name &name,
                                                   sc_clock *driver_clk_sig)
    : sc_module(name) {

  // Instantiate driver
  driver_inst = new two_to_two_driver("driver");
  driver_inst->clk(*driver_clk_sig);
  driver_inst->rst(rst_sig);
  driver_inst->req_addr(req_addr_sig);
  driver_inst->req_type(req_type_sig);
  driver_inst->req_wdata(req_wdata_sig);
  driver_inst->req_valid(req_valid_sig);
  driver_inst->req_ready(req_ready_sig);
  driver_inst->received_responses(received_responses_sig);

  driver_inst->rst_2(rst_sig_2);
  driver_inst->req_addr_2(req_addr_sig_2);
  driver_inst->req_type_2(req_type_sig_2);
  driver_inst->req_wdata_2(req_wdata_sig_2);
  driver_inst->req_valid_2(req_valid_sig_2);
  driver_inst->req_ready_2(req_ready_sig_2);
  driver_inst->received_responses_2(received_responses_sig_2);
  std::cout << "Finished parsing simulation trace ..." << std::endl;

  // Instantiate design top-level
  top_inst = new two_to_two_top("dut");
  std::cout << "Finished parsing simulation trace ...-----------------" << std::endl;
  top_inst->rst(rst_sig);
  top_inst->req_addr(req_addr_sig);
  top_inst->req_type(req_type_sig);
  top_inst->req_wdata(req_wdata_sig);
  top_inst->req_valid(req_valid_sig);
  top_inst->req_ready(req_ready_sig);
  top_inst->received_responses(received_responses_sig);
  std::cout << "Finished parsing simulation trace ..///////////////////////." << std::endl;

  top_inst->rst2(rst_sig_2);
  top_inst->req_addr2(req_addr_sig_2);
  top_inst->req_type2(req_type_sig_2);
  top_inst->req_wdata2(req_wdata_sig_2);
  top_inst->req_valid2(req_valid_sig_2);
  top_inst->req_ready2(req_ready_sig_2);
  top_inst->received_responses2(received_responses_sig_2);
}

two_to_two_system::~two_to_two_system() {
  delete driver_inst;
  delete top_inst;
  delete sysclk;
}