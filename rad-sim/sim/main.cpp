#include <design_context.hpp>
#include <fstream>
#include <iostream>
#include <radsim_config.hpp>
#include <sstream>
#include <systemc.h>

// INCLUDE YOUR SYSTEM HEADER FILE HERE
//#include <npu_system.hpp>
//#include <mlp_system.hpp>
//#include <mem_controller_test.hpp>
//#include <aximm_hello_world_system.hpp>
#include <dlrm_system.hpp>

RADSimConfig radsim_config;
RADSimDesignContext radsim_design;
std::ostream *gWatchOut;
SimLog sim_log;
SimTraceRecording sim_trace_probe;

int sc_main(int argc, char *argv[]) {
  gWatchOut = &cout;
  int log_verbosity = radsim_config.GetIntKnob("log_verbosity");
  sim_log.SetLogSettings(log_verbosity, "sim.log");

  int num_traces = radsim_config.GetIntKnob("num_traces");
  int num_trace_modules = radsim_config.GetIntKnob("num_trace_modules");
  sim_trace_probe.SetTraceRecordingSettings("sim.trace", num_traces,
                                            num_trace_modules);

  sc_clock *driver_clk_sig = new sc_clock(
      "node_clk0", radsim_config.GetDoubleKnob("sim_driver_period"), SC_NS);

  // INSTANTIATE YOUR SYSTEM DESIGN HERE
  // mlp_system *system = new mlp_system("mlp_system", driver_clk_sig);
  // npu_system* system = new npu_system("npu_system", driver_clk_sig);
  // mem_controller_system *system =
  //    new mem_controller_system("mem_controller_system");
  // aximm_hello_world_system *system =
  //    new aximm_hello_world_system("aximm_hello_world_system",
  //    driver_clk_sig);
  dlrm_system *system = new dlrm_system("dlrm_system", driver_clk_sig);
  sc_start();

  // system->dut_inst->PrintStats();
  delete system;
  delete driver_clk_sig;
  sc_flit scf;
  scf.FreeAllFlits();
  Flit *f = Flit::New();
  f->FreeAll();
  Credit *c = Credit::New();
  c->FreeAll();
  sim_trace_probe.dump_traces();
  (void)argc;
  (void)argv;
  return 0;
}
