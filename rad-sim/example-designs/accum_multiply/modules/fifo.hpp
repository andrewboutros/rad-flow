#pragma once

#include <queue>
#include <radsim_telemetry.hpp>
#include <sim_utils.hpp>
#include <systemc.h>
#include <vector>

// This code is copied from the dlrm example
// Define the fifo as fifo<dtype> *fifo_name; where dtype is the storage type
// one example would be fifo<uint64_t> *ififo;

// This class defines a vector FIFO module. This is a "peek" FIFO where the read
// data port always shows the top of the FIFO and the read enable signal is an
// acknowledgement signal (equivalent to pop in a software queue)
// Data is read by rdata.read() to a data_vector type, if only need a single value, 
// the datatype of rdata.read()[0] is <dtype>
template <class dtype> class fifo : public sc_module {
private:
  unsigned int capacity; // Depth of the FIFO
  unsigned int dwidth;   // Width of the FIFO in number of vector elements
  unsigned int fifo_almost_empty_size,
      fifo_almost_full_size; // Occupancy when FIFO is considered almost
                             // full/empty
  std::queue<std::vector<dtype>> mem; // FIFO storage implemented as a C++ queue

public:
  // Input signals
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_in<bool> wen;
  sc_in<data_vector<dtype>> wdata;
  sc_in<bool> ren;
  // Output signals
  sc_out<data_vector<dtype>> rdata;
  sc_out<bool> full;
  sc_out<bool> almost_full;
  sc_out<bool> empty;
  sc_out<bool> almost_empty;

  fifo(const sc_module_name &name, unsigned int depth, unsigned int width,
       unsigned int almost_full_size, unsigned int almost_empty_size);
  ~fifo();

  bool not_full();
  unsigned int occupancy();

  void Tick();
  SC_HAS_PROCESS(fifo);
};