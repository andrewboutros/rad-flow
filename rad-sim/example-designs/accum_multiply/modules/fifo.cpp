#include <fifo.hpp>

// This code is copied from the dlrm example

template <class dtype>
fifo<dtype>::fifo(const sc_module_name &name, unsigned int depth,
                  unsigned int width, unsigned int almost_full_size,
                  unsigned int almost_empty_size)
    : sc_module(name), wen("wen"), wdata("wdata"), ren("ren"), rdata("rdata"),
      full("full"), almost_full("almost_full"), empty("empty"),
      almost_empty("almost_empty") {
  // Initialize FIFO object with data width, capacity, 
  // and definition of what is considered almost full or empty

  dwidth = width;
  capacity = depth;
  fifo_almost_full_size = almost_full_size;
  fifo_almost_empty_size = almost_empty_size;

  // Set clock and reset signal for SC_CTHREAD
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

template <class dtype> fifo<dtype>::~fifo() {}

template <class dtype> bool fifo<dtype>::not_full() {
  return mem.size() < capacity;
}

template <class dtype> unsigned int fifo<dtype>::occupancy() {
  return mem.size();
}

template <class dtype> void fifo<dtype>::Tick() {
  // Reset logic
  // Remove all elements in FIFO
  while (!mem.empty())
    mem.pop();
  // Declare empty
  empty.write(true);
  almost_empty.write(true);
  // Declare not full
  full.write(false);
  almost_full.write(false);
  wait();

  // Sequential logic
  // Perform ren check - pop data if possible or raise error if empty
  // Perform wen operation - push data or raise error if full
  // Update status signals (almost) full/empty
  // Update rdata to be mem.front()
  while (true) {
    // Pop from queue if read enable signal is triggered and there is data in
    // the FIFO -- Data can always be read, but ren signals pop()
    if (ren.read()) {
      if (mem.size() == 0)
        sim_log.log(error, "FIFO is underflowing!", this->name());
      mem.pop();
    }

    // Push data into the FIFO if there is enough space
    if (wen.read()) {
      if (mem.size() == capacity)
        sim_log.log(error, "FIFO is overflowing!", this->name());
      data_vector<dtype> wdata_temp = wdata.read();
      std::vector<dtype> temp(wdata_temp.size());
      for (unsigned int element_id = 0; element_id < wdata_temp.size();
           element_id++){
        temp[element_id] = wdata_temp[element_id];
        std::cout << "fifo received " << (wdata_temp[element_id]) << endl;}
      mem.push(temp);
    }

    // Update FIFO status signals
    empty.write(mem.empty());
    almost_empty.write(mem.size() <= fifo_almost_empty_size);
    full.write(mem.size() == capacity);
    almost_full.write(mem.size() >= fifo_almost_full_size);

    // Set FIFO read data output to the top of the queue -- a vector of zeros is
    // produced if the queue is empty
    if (mem.size() == 0) {
      std::vector<dtype> temp(dwidth, 0);
      rdata.write(data_vector<dtype>(temp));
    } else {
      std::vector<dtype> temp = mem.front();
      rdata.write(data_vector<dtype>(temp));
    }

    wait();
  }
}

template class fifo<int16_t>;
template class fifo<sc_int<32>>;
template class fifo<sc_int<5>>;
template class fifo<sc_uint<5>>;