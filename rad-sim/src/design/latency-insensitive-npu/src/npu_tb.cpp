#include <npu_tb.hpp>

npu_tb::npu_tb(const sc_module_name &name)
    : sc_module(name),
      mrf_wdata("mrf_wdata"),
      ififo_rdy("ififo_rdy"),
      ififo_wen("ififo_wen"),
      ififo_wdata("ififo_wdata"),
      ofifo_rdy("ofifo_rdy"),
      ofifo_ren("ofifo_ren"),
      ofifo_rdata("ofifo_rdata") {
  start_cycle = 0;
  end_cycle = 0;

  init_vector<sc_in<bool>>::init_sc_vector(ififo_rdy, THREADS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(ififo_wen, THREADS, CORES);
  init_vector<sc_out<data_vector<tb_input_precision>>>::init_sc_vector(ififo_wdata, THREADS, CORES);
  init_vector<sc_in<bool>>::init_sc_vector(ofifo_rdy, THREADS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(ofifo_ren, THREADS, CORES);
  init_vector<sc_in<data_vector<tb_output_precision>>>::init_sc_vector(ofifo_rdata, THREADS, CORES);

  SC_METHOD(assign);
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    for (unsigned int core_id = 0; core_id < CORES; core_id++) sensitive << ofifo_rdy[thread_id][core_id];
  }
  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.neg());
}

npu_tb::~npu_tb() = default;

void npu_tb::source() {
  bool parse_flag;

  // Parse NPU instructions
  std::string inst_filename = "register_files/instructions.txt";
  std::string inst_path = NPU_ROOTDIR + inst_filename;
  parse_flag = ParseNPUInst(inst_path, npu_program);
  if (!parse_flag) {
    std::cerr << "Cannot parse instructions file!" << std::endl;
    exit(1);
  }

  // Parse inputs
  std::string inputs_filename = "register_files/inputs.txt";
  std::string inputs_path = NPU_ROOTDIR + inputs_filename;
  parse_flag = ParseNPUInputs(inputs_path, npu_inputs);
  if (!parse_flag) {
    std::cerr << "Cannot parse inputs file!" << std::endl;
    exit(1);
  }

  // Parse golden results
  std::string outputs_filename = "register_files/outputs.txt";
  std::string outputs_path = NPU_ROOTDIR + outputs_filename;
  parse_flag = ParseNPUOutputs(outputs_path, npu_outputs);
  if (!parse_flag) {
    std::cerr << "Cannot parse outputs file!" << std::endl;
    exit(1);
  }

  // Reset
  rst.write(true);
  inst_wen.write(false);
  start_pc.write(0);
  end_pc.write(npu_program.size());
  start.write(false);
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    for (unsigned int core_id = 0; core_id < CORES; core_id++) ififo_wen[thread_id][core_id].write(false);
  }
  wait();
  rst.write(false);
  wait();

  // Write NPU instructions
  for (unsigned int inst_id = 0; inst_id < npu_program.size(); inst_id++) {
    inst_wdata.write(npu_program[inst_id]);
    inst_wen.write(true);
    inst_waddr.write(inst_id);
    wait();
  }
  inst_wen.write(false);
  wait();
  std::cout << "Finished writing " << npu_program.size() << " NPU instructions!" << std::endl;

  // Trigger NPU start signal
  start.write(true);
  wait();
  start_cycle = GetNPUSimCycle(NPU_CLOCK_PERIOD);
  start.write(false);
  wait();

  // Write NPU inputs
  std::vector<unsigned int> num_written_inputs(THREADS, 0);
  bool all_threads_wrote_all_inputs = false;
  while (!all_threads_wrote_all_inputs) {
    all_threads_wrote_all_inputs = true;
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
      if (num_written_inputs[thread_id] == npu_inputs.size()) {
        for (unsigned int core_id = 0; core_id < CORES; core_id++) ififo_wen[thread_id][core_id].write(false);
        continue;
      } else {
        all_threads_wrote_all_inputs = false;
      }

      bool all_cores_ready = true;
      for (unsigned int core_id = 0; core_id < CORES; core_id++)
        all_cores_ready = all_cores_ready && ififo_rdy[thread_id][core_id].read();
      if (all_cores_ready) {
        for (unsigned int core_id = 0; core_id < CORES; core_id++) {
          ififo_wen[thread_id][core_id].write(true);
          ififo_wdata[thread_id][core_id].write(
              data_vector<tb_input_precision>(npu_inputs[num_written_inputs[thread_id]]));
        }
        num_written_inputs[thread_id]++;
      } else {
        for (unsigned int core_id = 0; core_id < CORES; core_id++) ififo_wen[thread_id][core_id].write(false);
      }
    }
    wait();
  }
  std::cout << "Finished writing " << npu_inputs.size() << " inputs!" << std::endl;
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    for (unsigned int core_id = 0; core_id < CORES; core_id++) ififo_wen[thread_id][core_id].write(false);
  }
  wait();
}

void npu_tb::sink() {
  std::vector<int> num_received_outputs(THREADS, 0);
  bool results_matching = true, mistake = false;

  bool all_threads_received_all_outputs = false;
  while (!all_threads_received_all_outputs) {
    all_threads_received_all_outputs = true;
    for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
      if (num_received_outputs[thread_id] == npu_outputs.size())
        continue;
      else
        all_threads_received_all_outputs = false;

      if (ofifo_ren[thread_id][0].read()) {
        for (unsigned int core_id = 0; core_id < CORES; core_id++) {
          mistake = false;
          data_vector<tb_output_precision> temp = ofifo_rdata[thread_id][core_id].read();
          for (unsigned int element_id = 0; element_id < LANES; element_id++) {
            results_matching &= (temp[element_id] == npu_outputs[num_received_outputs[thread_id]][element_id]);
            mistake |= (temp[element_id] != npu_outputs[num_received_outputs[thread_id]][element_id]);
          }
        }
        if (mistake) {
          cout << "Thread " << thread_id << " output " << num_received_outputs[thread_id] << " is mismatching" << endl;
          cout << "Got: ";
          cout << ofifo_rdata[thread_id][0].read() << " ";
          cout << endl;
          cout << "Expected: ";
          for (unsigned int element_id = 0; element_id < LANES; element_id++) {
            cout << npu_outputs[num_received_outputs[thread_id]][element_id] << " ";
          }
          cout << endl;
          cout << "+++++++++" << endl;
        }
        num_received_outputs[thread_id]++;
      }
    }
    wait();
  }
  end_cycle = GetNPUSimCycle(NPU_CLOCK_PERIOD);

  std::ofstream report;
  std::string report_filename = "sim_done";
  std::string report_path = NPU_ROOTDIR + report_filename;
  report.open(report_path);

  if (results_matching) {
    cout << "Simulation succeeded!" << endl;
    report << "PASS" << endl;
  } else {
    cout << "Simulation failed!" << endl;
    report << "FAIL" << endl;
  }
  cout << "Total simulation cycles = " << (end_cycle - start_cycle) << endl;
  report << (end_cycle - start_cycle) << endl;

  npu_trace_probe.record_event(TILES + 4, LAST_UOP_RETIRE_TRACE);
  npu_trace_probe.dump_traces();

  sc_stop();
}

void npu_tb::assign() {
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    for (unsigned int core_id = 0; core_id < CORES; core_id++)
      ofifo_ren[thread_id][core_id].write(ofifo_rdy[thread_id][core_id].read());
  }
}

npu_system::npu_system(const sc_module_name &name)
    : sc_module(name),
      mrf_wdata("mrf_wdata"),
      ififo_rdy("ififo_rdy"),
      ififo_wen("ififo_wen"),
      ififo_wdata("ififo_wdata"),
      ofifo_rdy("ofifo_rdy"),
      ofifo_ren("ofifo_ren"),
      ofifo_rdata("ofifo_rdata"),
      inst_wdata("inst_wdata") {

  init_vector<sc_signal<bool>>::init_sc_vector(ififo_rdy, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ififo_wen, THREADS, CORES);
  init_vector<sc_signal<data_vector<tb_input_precision>>>::init_sc_vector(ififo_wdata, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ofifo_rdy, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ofifo_ren, THREADS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(ofifo_rdata, THREADS, CORES);

  // Create clocks
  clk_sig = new sc_clock("node_clk", NPU_CLOCK_PERIOD, SC_NS);

  _tb = new npu_tb("TB");
  _tb->clk(*clk_sig);
  _tb->rst(rst_sig);
  _tb->inst_wdata(inst_wdata);
  _tb->inst_waddr(inst_waddr);
  _tb->inst_wen(inst_wen);
  _tb->start_pc(start_pc);
  _tb->end_pc(end_pc);
  _tb->start(start);
  _tb->mrf_waddr(mrf_waddr);
  _tb->mrf_wdata(mrf_wdata);
  _tb->mrf_wid(mrf_wid);
  _tb->ififo_rdy(ififo_rdy);
  _tb->ififo_wen(ififo_wen);
  _tb->ififo_wdata(ififo_wdata);
  _tb->ofifo_rdy(ofifo_rdy);
  _tb->ofifo_ren(ofifo_ren);
  _tb->ofifo_rdata(ofifo_rdata);

  _npu = new npu("NPU");
  _npu->clk(*clk_sig);
  _npu->rst(rst_sig);
  _npu->inst_wdata(inst_wdata);
  _npu->inst_waddr(inst_waddr);
  _npu->inst_wen(inst_wen);
  _npu->start_pc(start_pc);
  _npu->end_pc(end_pc);
  _npu->start(start);
  _npu->mrf_waddr(mrf_waddr);
  _npu->mrf_wdata(mrf_wdata);
  _npu->mrf_wid(mrf_wid);
  _npu->ififo_rdy(ififo_rdy);
  _npu->ififo_wen(ififo_wen);
  _npu->ififo_wdata(ififo_wdata);
  _npu->ofifo_rdy(ofifo_rdy);
  _npu->ofifo_ren(ofifo_ren);
  _npu->ofifo_rdata(ofifo_rdata);
}

npu_system::~npu_system() {
  delete clk_sig;
  delete _tb;
  delete _npu;
}
