#pragma once

#include <afifo.hpp>
#include <aximm_interface.hpp>
#include <design_context.hpp>
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

class feature_interaction : public radsim_module {
private:
  unsigned int _fifos_depth; // Depth of input/output FIFOs
  std::vector<feature_interaction_inst> _instructions; // Instruction mem
  sc_signal<unsigned int> _pc;                         // Program counter

  std::vector<afifo<int16_t> *> _input_fifos; // Input FIFOs
  sc_vector<sc_signal<data_vector<int16_t>>> _ififo_wdata, _ififo_rdata;
  sc_vector<sc_signal<bool>> _ififo_wen, _ififo_ren; // Read/Write Enable
  sc_vector<sc_signal<bool>> _ififo_full;            // Signals FIFOs full
  sc_vector<sc_signal<bool>> _ififo_afull; // Signals iFIFOs almost full
  sc_vector<sc_signal<bool>> _ififo_empty; // Signals iFIFOs empty

  afifo<int16_t> *_output_fifo; // Output FIFO
  sc_signal<data_vector<int16_t>> _ofifo_wdata, _ofifo_rdata;
  sc_signal<bool> _ofifo_wen, _ofifo_ren; // Read/Write Enable
  sc_signal<bool> _ofifo_full;            // Signals oFIFO full
  sc_signal<bool> _ofifo_afull;           // Signals oFIFO almost full
  sc_signal<bool> _ofifo_empty;           // Signals oFIFO empty

  unsigned int _num_mem_channels; // No. of memory channels
  unsigned int _dataw;            // Data interface bitwidth
  unsigned int _num_received_responses;
  unsigned int _num_elements;
  unsigned int _bitwidth;

public:
  sc_in<bool> rst;
  // Interface to driver logic
  sc_out<data_vector<int16_t>> feature_interaction_odata;
  sc_out<bool> feature_interaction_valid;
  sc_in<bool> feature_interaction_ready;
  sc_out<unsigned int> received_responses;
  // Interface to the NoC
  sc_vector<aximm_master_port> aximm_interface;

  feature_interaction(const sc_module_name &name, unsigned int dataw,
                      unsigned int element_bitwidth,
                      unsigned int num_mem_channels, unsigned int fifos_depth,
                      std::string &instructions_file);
  ~feature_interaction();

  void Assign();   // Combinational logic process
  void AssignPC(); // Combinational logic process
  void Tick();     // Sequential logic process
  void bv_to_data_vector(sc_bv<AXI_MAX_DATAW> &bitvector,
                         data_vector<int16_t> &datavector);
  SC_HAS_PROCESS(feature_interaction);
  void RegisterModuleInfo();
};