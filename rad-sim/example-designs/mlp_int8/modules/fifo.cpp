#include "fifo.hpp"

template <class dtype>
fifo<dtype>::fifo(const sc_module_name& name, unsigned int depth, unsigned int almost_full_size)
    : sc_module(name),
      push("push"),
      idata("idata"),
      pop("pop"),
      odata("odata"),
      full("full"),
      almost_full("almost_full"),
      empty("empty") {
        
  capacity = depth;
  fifo_almost_full_size = almost_full_size;

  // Set clock and reset signal for SC_CTHREAD
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

template <class dtype>
fifo<dtype>::~fifo() {}

template <class dtype>
void fifo<dtype>::Tick() {
  // Reset logic
  while (!mem.empty()) mem.pop();
  empty.write(true);
  full.write(false);
  almost_full.write(false);
  wait();

  // Sequential logic
  while (true) {
    // Pop from queue if read enable signal is triggered and there is data in the FIFO
    if (pop.read()) {
      if (mem.size() == 0) sim_log.log(error, "FIFO is underflowing!", this->name());
      mem.pop();
    }

    // Push data into the FIFO if there is enough space
    if (push.read()) {
      if (mem.size() == capacity) sim_log.log(error, "FIFO is overflowing!", this->name());
      mem.push(idata.read());
    }

    // Update FIFO status signals
    empty.write(mem.empty());
    full.write(mem.size() == capacity);
    almost_full.write(mem.size() >= fifo_almost_full_size);

    // Set FIFO read data output to the top of the queue -- a vector of zeros is produced if the queue is empty
    if (mem.size() > 0) odata.write(mem.front());

    wait();
  }
}

template class fifo<data_vector<sc_int<IPRECISION>>>;
template class fifo<mvm_inst>;
template class fifo<sc_uint<RF_ADDRW>>;
template class fifo<uint8_t>;