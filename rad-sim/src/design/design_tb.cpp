#include <design_tb.hpp>

design_driver::design_driver(const sc_module_name &name)
    : sc_module(name),
      mrf_wdata("mrf_wdata"),
      ififo_rdy("ififo_rdy"),
      ififo_wen("ififo_wen"),
      ififo_wdata("ififo_wdata"),
      ofifo_rdy("ofifo_rdy"),
      ofifo_ren("ofifo_ren"),
      ofifo_rdata("ofifo_rdata") {
  init_vector<sc_in<bool>>::init_sc_vector(ififo_rdy, THREADS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(ififo_wen, THREADS, CORES);
  init_vector<sc_out<data_vector<tb_input_precision>>>::init_sc_vector(ififo_wdata, THREADS, CORES);
  init_vector<sc_in<bool>>::init_sc_vector(ofifo_rdy, THREADS, CORES);
  init_vector<sc_out<bool>>::init_sc_vector(ofifo_ren, THREADS, CORES);
  init_vector<sc_in<data_vector<tb_output_precision>>>::init_sc_vector(ofifo_rdata, THREADS, CORES);

  start_cycle = 0;
  end_cycle = 0;

  SC_METHOD(assign);
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    for (unsigned int core_id = 0; core_id < CORES; core_id++) sensitive << ofifo_rdy[thread_id][core_id];
  }
  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.neg());
}

design_driver::~design_driver() {}

void design_driver::source() {
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
  start_cycle = GetNPUSimCycle(FABRIC_PERIOD);
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

void design_driver::sink() {
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

  NoCTransactionTelemetry::DumpStatsToFile("/Users/andrew/PhD/dev/rad-sim-opt-npu-multithread-hard-c2/stats.csv");
}

void design_driver::assign() {
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    for (unsigned int core_id = 0; core_id < CORES; core_id++) 
      ofifo_ren[thread_id][core_id].write(ofifo_rdy[thread_id][core_id].read());
  }
}

design_tb::design_tb(const sc_module_name &name, std::string &noc_config_filename)
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
  noc_clk_sig = new sc_clock("noc_clk", NOC_PERIOD, SC_NS);
  adapter_clk_sig = new sc_clock("adapter_clk", ADAPTER_PERIOD, SC_NS);
  node_clks_sig.resize(NUM_NODE_CLKS);
  node_clks_sig[0] = new sc_clock("node_clk0", FABRIC_PERIOD, SC_NS);
  node_clks_sig[1] = new sc_clock("node_clk1", BASEDIE_PERIOD, SC_NS);

  driver = new design_driver("driver");
  driver->clk(*node_clks_sig[0]);
  driver->rst(rst_sig);
  driver->inst_wdata(inst_wdata);
  driver->inst_waddr(inst_waddr);
  driver->inst_wen(inst_wen);
  driver->start_pc(start_pc);
  driver->end_pc(end_pc);
  driver->start(start);
  driver->mrf_waddr(mrf_waddr);
  driver->mrf_wdata(mrf_wdata);
  driver->mrf_wid(mrf_wid);
  driver->ififo_rdy(ififo_rdy);
  driver->ififo_wen(ififo_wen);
  driver->ififo_wdata(ififo_wdata);
  driver->ofifo_rdy(ofifo_rdy);
  driver->ofifo_ren(ofifo_ren);
  driver->ofifo_rdata(ofifo_rdata);

  rad_npu = new design("rad_npu", noc_config_filename, node_clks_sig);
  rad_npu->noc_clk(*noc_clk_sig);
  rad_npu->adapter_clk(*adapter_clk_sig);
  rad_npu->rst(rst_sig);
  rad_npu->inst_wdata(inst_wdata);
  rad_npu->inst_waddr(inst_waddr);
  rad_npu->inst_wen(inst_wen);
  rad_npu->start_pc(start_pc);
  rad_npu->end_pc(end_pc);
  rad_npu->start(start);
  rad_npu->mrf_waddr(mrf_waddr);
  rad_npu->mrf_wdata(mrf_wdata);
  rad_npu->mrf_wid(mrf_wid);
  rad_npu->ififo_rdy(ififo_rdy);
  rad_npu->ififo_wen(ififo_wen);
  rad_npu->ififo_wdata(ififo_wdata);
  rad_npu->ofifo_rdy(ofifo_rdy);
  rad_npu->ofifo_ren(ofifo_ren);
  rad_npu->ofifo_rdata(ofifo_rdata);
}

design_tb::~design_tb() {
  delete noc_clk_sig;
  delete adapter_clk_sig;
  for (unsigned int clk_id = 0; clk_id < NUM_NODE_CLKS; clk_id++) delete node_clks_sig[clk_id];
  delete driver;
  delete rad_npu;
}
