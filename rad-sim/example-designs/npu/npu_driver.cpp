#include <npu_driver.hpp>

npu_driver::npu_driver(const sc_module_name &name, RADSimDesignContext* radsim_design_)
    : sc_module(name),
      rst("rst"),
      inst_wdata("inst_wdata"),
      inst_waddr("inst_waddr"),
      inst_wen("inst_wen"),
      start_pc("start_pc"),
      end_pc("end_pc"),
      start("start"),
      mrf_waddr("mrf_waddr"),
      mrf_wdata("mrf_wdata"),
      mrf_wid("mrf_wid"),
      ififo_rdy("ififo_rdy"),
      ififo_wen("ififo_wen"),
      ififo_wdata("ififo_wdata"),
      ofifo_rdy("ofifo_rdy"),
      ofifo_ren("ofifo_ren"),
      ofifo_rdata("ofifo_rdata") {

  this->radsim_design = radsim_design_;

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

npu_driver::~npu_driver() {}

void npu_driver::source() {
  bool parse_flag;
  std::string npu_dir = radsim_config.GetStringKnob("radsim_user_design_root_dir");

  // Parse NPU instructions
  std::string inst_filename = "/register_files/instructions.txt";
  std::string inst_path = npu_dir + inst_filename;
  parse_flag = ParseNPUInst(inst_path, npu_program);
  if (!parse_flag) {
    std::cerr << "Cannot parse instructions file!" << std::endl;
    exit(1);
  }

  // Parse inputs
  std::string inputs_filename = "/register_files/inputs.txt";
  std::string inputs_path = npu_dir + inputs_filename;
  parse_flag = ParseNPUInputs(inputs_path, npu_inputs);
  if (!parse_flag) {
    std::cerr << "Cannot parse inputs file!" << std::endl;
    exit(1);
  }

  // Parse golden results
  std::string outputs_filename = "/register_files/outputs.txt";
  std::string outputs_path = npu_dir + outputs_filename;
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
  start_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));
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

void npu_driver::sink() {
  std::vector<unsigned int> num_received_outputs(THREADS, 0);
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
  end_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));

  std::ofstream report;
  std::string npu_dir = radsim_config.GetStringKnob("radsim_user_design_root_dir");
  std::string report_filename = "/sim_done";
  std::string report_path = npu_dir + report_filename;
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

  sim_trace_probe.dump_traces();

  //sc_stop();
  this->radsim_design->set_rad_done(); //flag to replace sc_stop calls
  return;

  //NoCTransactionTelemetry::DumpStatsToFile("/Users/andrew/PhD/dev/rad-sim-opt-npu-multithread-hard-c2/stats.csv");
}

void npu_driver::assign() {
  for (unsigned int thread_id = 0; thread_id < THREADS; thread_id++) {
    for (unsigned int core_id = 0; core_id < CORES; core_id++) 
      ofifo_ren[thread_id][core_id].write(ofifo_rdy[thread_id][core_id].read());
  }
}
