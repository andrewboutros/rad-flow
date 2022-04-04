#include "instruction_fifo.hpp"

template <class dtype>
instruction_fifo<dtype>::instruction_fifo(const sc_module_name& name, unsigned int depth, unsigned int almost_full_size,
                                          unsigned int almost_empty_size)
    : sc_module(name) {

  capacity = depth;
  fifo_almost_full_size = almost_full_size;
  fifo_almost_empty_size = almost_empty_size;
  reset_inst = sc_signal<dtype>();

  // Set clock and reset signal for SC_CTHREAD
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

template <class dtype>
instruction_fifo<dtype>::~instruction_fifo() {};

template <class dtype>
void instruction_fifo<dtype>::Tick() {
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
        npu_log.log(error, "Instruction FIFO is underflowing!", this->name());
      mem.pop();
    }

    // Push data into the FIFO if there is enough space
    if (wen.read()) {
      if (mem.size() > fifo_almost_full_size) 
        npu_log.log(error, "Instruction FIFO is overflowing!", this->name());
      mem.push(wdata.read());
    }

    // Update FIFO status signals
    empty.write(mem.empty());
    almost_empty.write(mem.size() <= fifo_almost_empty_size);
    full.write(mem.size() == capacity);
    almost_full.write(mem.size() >= fifo_almost_full_size);

    // Set FIFO read data output to the top of the queue -- a dummy instruction is produced if the queue is empty
    if (mem.size() == 0) {
      rdata.write(reset_inst);
    } else {
      rdata.write(mem.front());
    }

    //if (!mem.empty())
    //  cout << this->name() << ": " << mem.size() << endl;

    wait();
  }
}

template class instruction_fifo<mvu_uop>;
template class instruction_fifo<evrf_uop>;
template class instruction_fifo<mfu_uop>;
template class instruction_fifo<ld_uop>;
template class instruction_fifo<mvu_mop>;
template class instruction_fifo<evrf_mop>;
template class instruction_fifo<mfu_mop>;
template class instruction_fifo<ld_mop>;