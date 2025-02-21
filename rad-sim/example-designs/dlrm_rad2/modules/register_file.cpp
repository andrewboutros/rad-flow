#include <register_file.hpp>

template <class dtype>
register_file<dtype>::register_file(const sc_module_name &name, unsigned int id,
                                    unsigned int depth, unsigned int width,
                                    std::string &init_file)
    : sc_module(name), wdata_pipeline("wdata_pipeline", RF_WR_LATENCY),
      waddr_pipeline("waddr_pipeline", RF_WR_LATENCY),
      raddr_pipeline("raddr_pipeline", RF_RD_LATENCY),
      wen_pipeline("wen_pipeline", RF_WR_LATENCY), rst("rst"), raddr("raddr"),
      wdata("wdata"), waddr("waddr"), wen("wen"), clk_en("clk_en"),
      rdata("rdata") {

  register_file_id = id;
  num_elements_per_word = width;
  mem.resize(depth);
  for (unsigned int i = 0; i < depth; i++) {
    mem[i].resize(width);
  }

  if (!init_file.empty()) {
    bool parse =
        parse_register_file_contents_from_file(mem, init_file, width, depth);
    if (!parse) {
      cerr << "Error parsing contents of RF " << register_file_id << endl;
      exit(1);
    }
  }

  SC_METHOD(Assign);
  sensitive << wdata_pipeline[RF_WR_LATENCY - 1]
            << wen_pipeline[RF_WR_LATENCY - 1]
            << waddr_pipeline[RF_WR_LATENCY - 1]
            << raddr_pipeline[RF_WR_LATENCY - 1];
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

template <class dtype> register_file<dtype>::~register_file() {
  for (unsigned int i = 0; i < mem.size(); i++)
    mem[i].clear();
}

template <class dtype> void register_file<dtype>::Assign() {
  // Assign read data output
  rdata.write(
      data_vector<dtype>(mem[raddr_pipeline[RF_RD_LATENCY - 1].read()]));

  // Assign write value
  if (wen_pipeline[RF_WR_LATENCY - 1].read()) {
    uint32_t addr = waddr_pipeline[RF_WR_LATENCY - 1].read();
    data_vector<dtype> temp = wdata_pipeline[RF_WR_LATENCY - 1].read();
    if (temp.size() != 0)
      for (unsigned int i = 0; i < num_elements_per_word; i++)
        mem[addr][i] = temp[i];
  }
}

template <class dtype> void register_file<dtype>::Tick() {
  for (unsigned int i = 0; i < RF_RD_LATENCY; i++) {
    raddr_pipeline[i].write(0);
  }
  for (unsigned int i = 0; i < RF_WR_LATENCY; i++) {
    waddr_pipeline[i].write(0);
    wen_pipeline[i].write(false);
  }
  wait();

  while (true) {
    if (!clk_en.read()) {
      // Address guards
      if (raddr.read() >= mem.size()) {
        cerr << "Read address (" << raddr.read() << ") out of bound at RF "
             << register_file_id << endl;
        exit(1);
      }
      if (waddr.read() >= mem.size()) {
        cerr << "Write address (" << waddr.read() << ") out of bound at RF "
             << register_file_id << endl;
        exit(1);
      }

      // Populate first stage of pipeline
      raddr_pipeline[0].write(raddr.read());
      wdata_pipeline[0].write(data_vector<dtype>(wdata));
      waddr_pipeline[0].write(waddr.read());
      wen_pipeline[0].write(wen.read());

      // Advance the rest of the pipeline
      for (unsigned int i = 1; i < RF_RD_LATENCY; i++) {
        raddr_pipeline[i].write(raddr_pipeline[i - 1]);
      }
      for (unsigned int i = 1; i < RF_WR_LATENCY; i++) {
        wdata_pipeline[i].write(wdata_pipeline[i - 1].read());
        waddr_pipeline[i].write(waddr_pipeline[i - 1].read());
        wen_pipeline[i].write(wen_pipeline[i - 1].read());
      }
    }
    wait();
  }
}

template class register_file<int16_t>;