#include <aximm_hello_world_top.hpp>

accum_multiply_top::accum_multiply_top(const sc_module_name &name)
    : sc_module(name) {

  std::string module_name_str;
  char module_name[25];

  module_name_str = "adder_inst";
  std::strcpy(module_name, module_name_str.c_str());

  adder_inst = new adder(module_name, 16, 16);
  adder_inst->rst(rst);
  adder_inst->input(input);
  adder_inst->input_valid(input_valid);
  adder_inst->input_ready(input_ready);

  module_name_str = "multiplier_inst";
  std::strcpy(module_name, module_name_str.c_str());
  multiplier_inst = new responder(module_name, 16, 16);
  responder_inst->rst(rst);
  multiplier_inst->output(output);
  multiplier_inst->output_ready(output_ready);
  multiplier_inst->output_valid(output_valid);

  radsim_design.BuildDesignContext("accum_multiply.place",
                                   "accum_multiply.clks");
  radsim_design.CreateSystemNoCs(rst);
  radsim_design.ConnectModulesToNoC();
}

accum_multiply_top::~accum_multiply_top() {
  delete adder_inst;
  delete multiplier_inst;
}