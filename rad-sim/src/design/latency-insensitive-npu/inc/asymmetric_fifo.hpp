#pragma once

#include <systemc.h>
#include <params.hpp>
#include <queue>
#include <sim_utils.hpp>
#include <vector>

// This class defines an asymmetric FIFO module where the number of input vector elements is an integer multiple of the
// number of output vector elements. This is a "peek" FIFO where the read data port always shows the top of the FIFO
// and the read enable signal is an acknowledgement signal (equivalent to pop in a software queue)
template <class dtype>
class asymmetric_fifo : public sc_module {
 private:
  int capacity;                                       // FIFO capcity in words of the smaller width (i.e., output words)
  int input_width, output_width;                      // Input and output widths in vector elemnts
  int width_ratio;                                    // Ratio between input and output widths
  int fifo_almost_empty_size, fifo_almost_full_size;  // Almost empty and almost full sizes
  std::queue<std::vector<dtype>> mem;                 // Memory of the FIFO (queue of vectors)

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
  sc_out<bool> almost_empty;

  asymmetric_fifo(const sc_module_name& name, int depth, int iwidth, int owidth, int almost_full_size,
                  int almost_empty_size);
  ~asymmetric_fifo();

  void Tick();
  SC_HAS_PROCESS(asymmetric_fifo);
};