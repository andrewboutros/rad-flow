#include "asymmetric_fifo.hpp"

template <class dtype>
asymmetric_fifo<dtype>::asymmetric_fifo(const sc_module_name& name, unsigned int depth, unsigned int iwidth, 
                                        unsigned int owidth, unsigned int almost_full_size, 
                                        unsigned int almost_empty_size) : sc_module(name) {
  // Setting input/output widths of the asymmetric FIFO (making sure input width is integer multiple of output width)
  if (iwidth % owidth != 0)
    sim_log.log(error, "Input width of asymmetric FIFO must be a multiple of its output width!", this->name());
  input_width = iwidth;
  output_width = owidth;
  width_ratio = (int)(input_width / output_width);

  // Setting the actual, almost full and almost empty depths of the FIFO
  capacity = depth;
  fifo_almost_full_size = almost_full_size;
  fifo_almost_empty_size = almost_empty_size;

  // Set SystemC cthread clock edge and reset signal
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

template <class dtype>
asymmetric_fifo<dtype>::~asymmetric_fifo() {}

template <class dtype>
void asymmetric_fifo<dtype>::Tick() {
  // Reset logic
  while (!mem.empty())
    mem.pop();
  empty.write(true);
  almost_empty.write(true);
  full.write(false);
  almost_full.write(false);
  wait();

  // Sequential logic
  while (true) {
    // Pop from queue if read enable signal is triggered and there is data in the FIFO
    if (ren.read()) {
      if (mem.size() == 0)
        sim_log.log(error, "FIFO is underflowing!", this->name());
      mem.pop();
    }

    // Push data into the FIFO if there is enough space
    if (wen.read()) {
      if (mem.size() > (capacity - width_ratio)) 
        sim_log.log(error, "FIFO is overflowing!", this->name());
      // The asymmetric nature of the FIFO is modeled by splitting an input and pushing multiple chunks of it to the 
      // queue in a given cycle. This is different than the typical hardware implementation in which the wide input
      // vector is pushed into a wide FIFO, and then smaller chunks of it are produced when FIFO is popped
      for (unsigned int chunk_id = 0; chunk_id < width_ratio; chunk_id++) {
        std::vector<dtype> temp(output_width);
        data_vector<dtype> wdata_temp = wdata.read();
        for (unsigned int element_id = 0; element_id < output_width; element_id++) 
          temp[element_id] = wdata_temp[(chunk_id * output_width) + element_id];
        mem.push(temp);
      }
    }

    // Update FIFO status signals
    empty.write(mem.empty());
    almost_empty.write(mem.size() <= fifo_almost_empty_size);
    full.write(mem.size() > (capacity - width_ratio));
    almost_full.write(mem.size() > (fifo_almost_full_size - width_ratio));

    // Set FIFO read data output to the top of the queue -- a vector of zeros is produced if the queue is empty
    if (mem.size() == 0) {
      std::vector<dtype> temp(output_width);
      rdata.write(data_vector<dtype>(temp));
    } else {
      std::vector<dtype> temp = mem.front();
      rdata.write(data_vector<dtype>(temp));
      assert(temp.size() == output_width);
    }
    wait();
  }
}

template class asymmetric_fifo<tb_output_precision>;