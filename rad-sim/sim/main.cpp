#include <design_context.hpp>
#include <fstream>
#include <iostream>
#include <radsim_config.hpp>
#include <sstream>
#include <systemc.h>

#include <mlp_int8_system.hpp>

RADSimConfig radsim_config;
RADSimDesignContext radsim_design;
std::ostream *gWatchOut;
SimLog sim_log;
SimTraceRecording sim_trace_probe;

int sc_main(int argc, char *argv[]) {
	gWatchOut = &cout;
	int log_verbosity = radsim_config.GetIntKnob("telemetry_log_verbosity");
	sim_log.SetLogSettings(log_verbosity, "sim.log");

	int num_traces = radsim_config.GetIntKnob("telemetry_num_traces");
	sim_trace_probe.SetTraceRecordingSettings("sim.trace", num_traces);

	sc_clock *driver_clk_sig = new sc_clock(
		"node_clk0", radsim_config.GetDoubleKnob("sim_driver_period"), SC_NS);

	mlp_int8_system *system = new mlp_int8_system("mlp_int8_system", driver_clk_sig);
	sc_start();

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
	return radsim_design.GetDesignResult();
}
