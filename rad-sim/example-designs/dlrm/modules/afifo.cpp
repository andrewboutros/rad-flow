#include <afifo.hpp>

template <class dtype>
afifo<dtype>::afifo(const sc_module_name &name, unsigned int depth,
                    unsigned int iwidth, unsigned int owidth,
                    unsigned int almost_full_size)
    : sc_module(name), _staging_vector(owidth) {

  _wide_to_narrow = (iwidth > owidth);
  _input_width = iwidth;
  _output_width = owidth;
  if (_wide_to_narrow) {
    _width_ratio = (int)(_input_width / _output_width);
  } else {
    _width_ratio = (int)(_output_width / _input_width);
  }
  _capacity = depth;
  _fifo_almost_full_size = almost_full_size;
  _staging_counter = 0;

  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

template <class dtype> afifo<dtype>::~afifo() {}

template <class dtype> void afifo<dtype>::Tick() {
  // Reset logic
  while (!_mem.empty())
    _mem.pop();
  empty.write(true);
  full.write(false);
  almost_full.write(false);
  wait();

  // Sequential logic
  while (true) {
    // Pop logic
    if (ren.read()) {
      if (_mem.size() == 0) {
        sim_log.log(error, "FIFO is underflowing!", this->name());
      }
      _mem.pop();
    }

    // Push logic
    data_vector<dtype> wdata_vector;
    if (wen.read()) {
      if (_wide_to_narrow) {
        if (_mem.size() > _capacity - _width_ratio) {
          sim_log.log(error,
                      "FIFO is overflowing! Size = " +
                          std::to_string(_mem.size()),
                      this->name());
        }
        wdata_vector = wdata.read();
        for (unsigned int i = 0; i < _width_ratio; i++) {
          data_vector<dtype> tmp(_output_width);
          for (unsigned int j = 0; j < _output_width; j++) {
            tmp[j] = wdata_vector[(i * _output_width) + j];
          }
          _mem.push(tmp);
        }
      } else {
        wdata_vector = wdata.read();
        for (unsigned int i = 0; i < _input_width; i++) {
          _staging_vector[(_staging_counter * _input_width) + i] =
              wdata_vector[i];
        }
        if (_staging_counter == _width_ratio - 1) {
          if (_mem.size() >= _capacity) {
            sim_log.log(error,
                        "FIFO is overflowing! Size = " +
                            std::to_string(_mem.size()),
                        this->name());
          }
          _staging_counter = 0;
          _mem.push(_staging_vector);
        } else {
          _staging_counter++;
        }
      }
    }

    empty.write(_mem.size() == 0);
    if (_wide_to_narrow)
      full.write(_mem.size() > _capacity - _width_ratio);
    else
      full.write(_mem.size() >= _capacity);
    almost_full.write(_mem.size() > _capacity - 2 * _width_ratio);
    data_vector<dtype> tmp(_output_width);
    if (_mem.size() > 0) {
      tmp = _mem.front();
    }
    rdata.write(tmp);

    // std::cout << this->name() << ": " << _mem.size() << " "
    //           << (_mem.size() >= _fifo_almost_full_size) << std::endl;
    wait();
  }
}

template class afifo<int16_t>;