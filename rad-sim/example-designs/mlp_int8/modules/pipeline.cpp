#include "pipeline.hpp"

template <class dtype>
pipeline<dtype>::pipeline(const sc_module_name& name, unsigned int depth)
    : sc_module(name),
      idata("idata"),
      odata("odata") {
        
  pipeline_depth = depth;
  pipeline_reg.init(pipeline_depth);

  // Set clock and reset signal for SC_CTHREAD
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
  SC_METHOD(Assign);
  sensitive << pipeline_reg[pipeline_depth-1];
}

template <class dtype>
pipeline<dtype>::~pipeline() {}

template <class dtype>
void pipeline<dtype>::Tick() {
  // Reset logic
  wait();

  // Sequential logic
  while (true) {
    for (unsigned int i = pipeline_depth - 1; i > 0; i--) pipeline_reg[i].write(pipeline_reg[i-1].read());
    pipeline_reg[0].write(idata.read());

    wait();
  }
}

template <class dtype>
void pipeline<dtype>::Assign() {
  odata.write(pipeline_reg[pipeline_depth-1].read());
}

template class pipeline<data_vector<sc_int<IPRECISION>>>;
template class pipeline<mvm_inst>;
template class pipeline<bool>;
template class pipeline<sc_int<OPRECISION>>;