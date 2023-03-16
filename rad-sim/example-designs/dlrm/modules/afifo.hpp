#pragma once

#include <queue>
#include <radsim_telemetry.hpp>
#include <sim_utils.hpp>
#include <systemc.h>
#include <vector>

template <class dtype> class afifo : public sc_module {
private:
  unsigned int _capacity;              // FIFO capcity in narrower words
  unsigned int _input_width;           // Input width in vector elements
  unsigned int _output_width;          // Output widths in vector elemnts
  bool _wide_to_narrow;                // Flag to specify assymetry mode
  unsigned int _width_ratio;           // Ratio between input and output widths
  unsigned int _fifo_almost_full_size; // Almost full size
  std::queue<data_vector<dtype>> _mem; // Memory of the FIFO (queue of vectors)
  data_vector<dtype> _staging_vector;
  unsigned int _staging_counter;

public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_in<bool> wen;
  sc_in<data_vector<dtype>> wdata;
  sc_in<bool> ren;
  sc_out<data_vector<dtype>> rdata;
  sc_out<bool> full;
  sc_out<bool> almost_full;
  sc_out<bool> empty;

  afifo(const sc_module_name &name, unsigned int depth, unsigned int iwidth,
        unsigned int owidth, unsigned int almost_full_size);
  ~afifo();

  void Tick();
  SC_HAS_PROCESS(afifo);
};