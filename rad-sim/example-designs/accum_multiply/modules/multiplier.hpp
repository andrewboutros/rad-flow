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

// This module receives data from the axis interface, and constantly multiply the head of fifo with the value stored in register
// The ififo's data will be popped into the register. Initial value is always 0. 
// ofifo receives the product as long as ififo isn't empty. (effectively, we use ififo head as a temp register)

class multiplier : public RADSimModule {
private:
  // Modules and Signals Only (supposed to be) Internally Accessible
  fifo<int16_t> *ififo; // Input FIFO
  sc_int<16> internal_register; // Storing the value to be multiplied with fifo head
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
  unsigned int ififo_depth; // Number of values that can be stored in ififo, assigned in constructor
  unsigned int ofifo_depth; // Number of values that can be stored in ofifo, assigned in constructor
  data_vector<int16_t> input_data_temp; // Temp data vector to convert values to data_vector
  data_vector<int16_t> output_data_temp; // Temp data vector to convert values to data_vector

public:
  // Signal accessible by external modules
  sc_in<bool> rst; // Reset Signal

  // Output side signals
  sc_out<int16_t> output;
  sc_in<bool> output_ready;
  sc_out<bool> output_valid;

  // Interface to the NoC
  axis_master_port axis_multiplier_interface;

  multiplier(const sc_module_name &name, unsigned int ififo_depth, unsigned int ofifo_depth);
  ~multiplier();

  void Assign(); // Combinational logic process
  void Tick();   // Sequential logic process
  SC_HAS_PROCESS(multiplier);
  void RegisterModuleInfo();
};