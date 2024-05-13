#pragma once

#include <aximm_interface.hpp>
#include <design_context.hpp>
#include <fifo.hpp>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>

#define DATAW 512

// This code is based on the requester from the aximm_hello_world example

// This module receives data from the driver, and sums values in groups of 4
// Then the data outputs the sum via ofifo
// Only data in groups of 4 will be summed, so the 3rd data is not summed with the 5th data
// Data is summed like: (1,2,3,4) (5,6,7,8)
// Numbers are input into a shift register before summed, sum is only recorded in period of 4

class adder : public RADSimModule {
private:
  // Modules and Signals Only (supposed to be) Internally Accessible
  fifo<int16_t> *ififo; // Input FIFO
  sc_int<16> internal_registers[4]; // Storing 4 numbers that is to be summed and outputted
  sc_signal<bool> sum_valid; // Signal flagging if all 4 numbers are ready (triggered by an internal counter)
  fifo<int16_t> *ofifo; // Output FIFO (here we ignore edge case of overflow from adding 4 numbers)

  // Internal Variables
  unsigned int num_values_received;

  // TODO
  // To write to ofifo:
  // ofifo.read_signal.write(data_vector<int16_t>)


public:
  // Signal accessible by external modules
  sc_in<bool> rst; // Reset Signal

  // Input side signals
  sc_in<sc_int<16>> input;
  sc_in<bool> input_valid;
  sc_out<bool> input_ready;

  // Output side signals
  sc_out<sc_int<16>> output;
  sc_in<bool> output_ready;
  sc_out<bool> output_valid;

  // Interface to the NoC
  axis_master_port axis_adder_interface;

  adder(const sc_module_name &name, unsigned int fifo_depth);
  ~adder();

  void Assign(); // Combinational logic process
  void Tick();   // Sequential logic process
  SC_HAS_PROCESS(adder);
  void RegisterModuleInfo();
};