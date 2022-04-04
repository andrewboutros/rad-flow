#include <piso_register_file.hpp>

template <class dtype>
piso_register_file<dtype>::piso_register_file(const sc_module_name& name, unsigned int id, unsigned int depth,
                                              unsigned int width, std::string& init_file)
    : sc_module(name),
      rdata("rdata"),
      wdata("wdata"),
      rdata_pipeline("rdata_pipeline", RF_RD_LATENCY),
      wdata_pipeline("wdata_pipeline", RF_WR_LATENCY),
      waddr_pipeline("waddr_pipeline", RF_WR_LATENCY),
      wen_pipeline("wen_pipeline", RF_WR_LATENCY) {
  register_file_id = id;
  full_width = width;
  if (width % (DPE_NUM_TBS - 1) != 0) {
    cerr << "PISO RF width must be divisible by number of tensor blocks per DPE!" << endl;
    exit(1);
  }
  num_elements_per_word = width / (DPE_NUM_TBS - 1);
  mem.resize(DPE_NUM_TBS - 1);
  for (unsigned int i = 0; i < DPE_NUM_TBS - 1; i++) {
    mem[i].resize(depth);
    for (unsigned int j = 0; j < depth; j++) mem[i][j].resize(width);
  }

  if (!init_file.empty()) {
    for (unsigned int i = 0; i < mem.size(); i++) {
      std::string init_file_name = init_file;
      bool parse = parse_register_file_contents_from_file(mem[i], init_file_name, num_elements_per_word, depth);
      if (!parse) {
        cerr << "Error parsing contents of PISO RF " << register_file_id << "_" << i << endl;
        exit(1);
      }
    }
  }

  SC_METHOD(Assign);
  sensitive << rdata_pipeline[RF_RD_LATENCY - 1] << wdata_pipeline[RF_WR_LATENCY - 1] << wen_pipeline[RF_WR_LATENCY - 1]
            << waddr_pipeline[RF_WR_LATENCY - 1];
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

template <class dtype>
piso_register_file<dtype>::~piso_register_file() = default;

template <class dtype>
void piso_register_file<dtype>::Assign() {
  // Assign read data output
  rdata.write(rdata_pipeline[RF_RD_LATENCY - 1].read());

  // Assign write value
  if (wen_pipeline[RF_WR_LATENCY - 1].read()) {
    uint32_t addr = waddr_pipeline[RF_WR_LATENCY - 1].read();
    data_vector<dtype> temp = wdata_pipeline[RF_WR_LATENCY - 1].read();
    for (unsigned int j = 0; j < DPE_NUM_TBS - 1; j++) {
      for (unsigned int i = 0; i < num_elements_per_word; i++) {
        mem[j][addr][i] = temp[(j * num_elements_per_word) + i];
      }
    }
#ifdef RF_PRINTOUT
    std::cout << "RF writing " << mem[0][addr][0] << " to address " << addr << std::endl;
#endif
  }
}

template <class dtype>
void piso_register_file<dtype>::Tick() {
  for (unsigned int i = 0; i < RF_WR_LATENCY; i++) {
    waddr_pipeline[i].write(0);
    wen_pipeline[i].write(false);
  }
  wait();

  while (true) {
    if (waddr.read() >= mem[0].size()) {
      cerr << "Write address out of bound at RF " << this->name() << " (" << waddr.read() << ")" << endl;
      exit(1);
    }
    wdata_pipeline[0].write(wdata.read());
    waddr_pipeline[0].write(waddr.read());
    wen_pipeline[0].write(wen.read());
    for (unsigned int i = 1; i < RF_WR_LATENCY; i++) {
      wdata_pipeline[i].write(wdata_pipeline[i - 1].read());
      waddr_pipeline[i].write(waddr_pipeline[i - 1].read());
      wen_pipeline[i].write(wen_pipeline[i - 1].read());
    }

    if (!clk_en.read()) {
      // Address guards
      if (raddr.read() >= mem[0].size()) {
        cerr << "Read address out of bound at RF " << this->name() << " (" << raddr.read() << ")" << endl;
        exit(1);
      }

      // Populate first stage of pipeline
      rdata_pipeline[0].write(data_vector<dtype>(mem[rd_id.read()][raddr.read()]));

      // Advance the rest of the pipeline
      for (unsigned int i = 1; i < RF_RD_LATENCY; i++) {
        rdata_pipeline[i].write(rdata_pipeline[i - 1].read());
      }
    }

    wait();
  }
}

template class piso_register_file<tb_input_precision>;