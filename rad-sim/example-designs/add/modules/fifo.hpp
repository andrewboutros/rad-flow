#pragma once

#include <systemc.h>
#include <queue>
#include <vector>
#include <radsim_telemetry.hpp>

#define DATAW 128

// This class defines a FIFO module. This is a "peek" FIFO where the read data port always shows the top of the
// FIFO and the read enable signal is an acknowledgement signal (equivalent to pop in a software queue)
template <class dtype>
class fifo : public sc_module {
 private:
  unsigned int capacity;                                       // Depth of the FIFO
  unsigned int fifo_almost_empty_size, fifo_almost_full_size;  // Occupancy when FIFO is considered almost full/empty
  std::queue<dtype> mem;                                       // FIFO storage implemented as a C++ queue

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_in<bool> wen;
  sc_in<dtype> wdata;
  sc_in<bool> ren;
  sc_out<dtype> rdata;
  sc_out<bool> full;
  sc_out<bool> almost_full;
  sc_out<bool> empty;
  sc_out<bool> almost_empty;

  fifo(const sc_module_name& name, unsigned int depth, unsigned int almost_full_size,
       unsigned int almost_empty_size);
  ~fifo();

  void Tick();
  SC_HAS_PROCESS(fifo);
};