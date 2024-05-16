#pragma once

#include <axis_interface.hpp>
#include <design_context.hpp>
#include <fifo.hpp>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>

#define DATAW 512
#define NUMSUM 4

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
  sc_int<16> internal_registers[NUMSUM]; // Storing 4 numbers that is to be summed and outputted
  sc_signal<bool> sum_valid; // Signal flagging if all 4 numbers are ready (triggered by an internal counter)
  fifo<int16_t> *ofifo; // Output FIFO (here we ignore edge case of overflow from adding 4 numbers)

  // FIFO status signals
  // These signals should've been modified in a system verilog equivalent of always_comb, but we just assign it in tick. 
  // These signals are same as dlrm example mvm
  sc_signal<data_vector<int16_t>> ififo_wdata_signal, ififo_rdata_signal;
  sc_signal<bool> ififo_wen_signal, ififo_ren_signal, ififo_full_signal,
      ififo_empty_signal, ififo_almost_full_signal, ififo_almost_empty_signal;
  sc_signal<data_vector<int16_t>> ofifo_wdata_signal, ofifo_rdata_signal;
  sc_signal<bool> ofifo_wen_signal, ofifo_ren_signal, ofifo_full_signal,
      ofifo_empty_signal, ofifo_almost_full_signal, ofifo_almost_empty_signal;

  // Internal Variables
  unsigned int num_values_received; // Counter for informing when the accumulated sum is correct
  unsigned int ififo_depth; // Number of values that can be stored in ififo, assigned in constructor
  unsigned int ofifo_depth; // Number of values that can be stored in ofifo, assigned in constructor
  data_vector<int16_t> input_data_temp, output_data_temp; // Temp data vector to convert values to data_vector
  unsigned int temp_sum; // used to store sum when output

public:
  // Signal accessible by external modules
  sc_in<bool> rst; // Reset Signal

  // Input side signals
  sc_in<int16_t> input;
  sc_in<bool> input_valid;
  sc_out<bool> input_ready;

  // Interface to the NoC
  axis_master_port axis_adder_interface;

  adder(const sc_module_name &name, unsigned int ififo_depth, unsigned int ofifo_depth);
  ~adder();

  void Assign(); // Combinational logic process
  void Tick();   // Sequential logic process
  SC_HAS_PROCESS(adder);
  void RegisterModuleInfo();
};