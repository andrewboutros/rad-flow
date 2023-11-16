#pragma once

#include <afifo.hpp>
#include <aximm_interface.hpp>
#include <axis_interface.hpp>
#include <design_context.hpp>
#include <mvm.hpp>
#include <queue>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <sim_utils.hpp>
#include <string>
#include <systemc.h>
#include <vector>

struct feature_interaction_inst {
  unsigned int mux_select;
  std::vector<bool> fifo_pops;
};

class feature_interaction : public RADSimModule {
private:
  unsigned int _fifos_depth; // Depth of input/output FIFOs
  unsigned int _afifo_width_ratio_in;
  unsigned int _afifo_width_ratio_out;
  std::vector<feature_interaction_inst> _instructions; // Instruction mem
  sc_signal<unsigned int> _pc;                         // Program counter

  std::vector<std::queue<data_vector<int16_t>>> _input_fifos; // Input FIFOs
  sc_vector<sc_signal<bool>> _ififo_full;  // Signals FIFOs full
  sc_vector<sc_signal<bool>> _ififo_empty; // Signals iFIFOs empty

  std::vector<std::queue<data_vector<int16_t>>> _output_fifos; // Output FIFO
  sc_vector<sc_signal<bool>> _ofifo_full;  // Signals oFIFO full
  sc_vector<sc_signal<bool>> _ofifo_empty; // Signals oFIFO empty
  sc_signal<unsigned int> _dest_ofifo, _src_ofifo;
  data_vector<int16_t> _staging_data;
  unsigned int _staging_counter;

  unsigned int _num_mem_channels; // No. of memory channels
  unsigned int _dataw;            // Data interface bitwidth
  unsigned int _num_received_responses;
  unsigned int _num_elements_wide_in;
  unsigned int _num_elements_narrow;
  unsigned int _num_elements_wide_out;
  unsigned int _bitwidth;
  unsigned int _num_output_channels;
  unsigned int _num_expected_responses;

  ofstream *_debug_feature_interaction_out;

public:
  sc_in<bool> rst;
  // Interface to driver logic
  sc_out<unsigned int> received_responses;
  // Interface to the NoC
  sc_vector<aximm_master_port> aximm_interface;
  sc_vector<axis_master_port> axis_interface;

  feature_interaction(const sc_module_name &name, unsigned int dataw,
                      unsigned int element_bitwidth,
                      unsigned int num_mem_channels, unsigned int fifos_depth,
                      unsigned int num_output_channels,
                      std::string &instructions_file);
  ~feature_interaction();

  void Assign(); // Combinational logic process
  void Tick();   // Sequential logic process
  void bv_to_data_vector(sc_bv<AXI4_MAX_DATAW> &bitvector,
                         data_vector<int16_t> &datavector,
                         unsigned int num_elements);
  void data_vector_to_bv(data_vector<int16_t> &datavector,
                         sc_bv<AXIS_MAX_DATAW> &bitvector,
                         unsigned int num_elements);
  SC_HAS_PROCESS(feature_interaction);
  void RegisterModuleInfo();
};