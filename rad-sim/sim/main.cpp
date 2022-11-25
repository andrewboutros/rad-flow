#include <systemc.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <radsim_config.hpp>
#include <design_context.hpp>
//#include <npu_system.hpp>
#include <mlp_system.hpp>

RADSimConfig radsim_config;
RADSimDesignContext radsim_design;
std::ostream* gWatchOut;
SimLog sim_log;
SimTraceRecording sim_trace_probe;

int sc_main(int argc, char* argv[]) {
  gWatchOut = &cout;
  int log_verbosity = radsim_config.GetIntKnob("log_verbosity");
  sim_log.SetLogSettings(log_verbosity, "sim.log");

  int num_traces = radsim_config.GetIntKnob("num_traces");
  int num_trace_modules = radsim_config.GetIntKnob("num_trace_modules");
  sim_trace_probe.SetTraceRecordingSettings("sim.trace", num_traces, num_trace_modules);

  sc_clock* driver_clk_sig = new sc_clock("node_clk0", radsim_config.GetDoubleKnob("sim_driver_period"), SC_NS);
  mlp_system* system = new mlp_system("mlp_system", driver_clk_sig);
  //npu_system* system = new npu_system("npu_system", driver_clk_sig);
  //radsim_design.DumpDesignContext();
  sc_start();

  delete system;
  delete driver_clk_sig;
  sc_flit scf; scf.FreeAllFlits();
  Flit* f = Flit::New(); f->FreeAll();
  Credit* c = Credit::New(); c->FreeAll();
  (void) argc; (void) argv;
  return 0;
}