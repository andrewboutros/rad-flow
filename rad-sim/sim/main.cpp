#include <design_context.hpp>
#include <fstream>
#include <iostream>
#include <radsim_config.hpp>
#include <sstream>
#include <systemc.h>

#include <add_system.hpp>

RADSimConfig radsim_config;
//RADSimDesignContext radsim_design; //AKB: commented out
std::ostream *gWatchOut;
SimLog sim_log;
SimTraceRecording sim_trace_probe;

int sc_main(int argc, char *argv[]) {
	RADSimDesignContext* radsim_design1 = new RADSimDesignContext(); //AKB: added
	RADSimDesignContext* radsim_design2 = new RADSimDesignContext(); //AKB: added
	gWatchOut = &cout;
	int log_verbosity = radsim_config.GetIntKnob("telemetry_log_verbosity");
	sim_log.SetLogSettings(log_verbosity, "sim.log");

	int num_traces = radsim_config.GetIntKnob("telemetry_num_traces");
	sim_trace_probe.SetTraceRecordingSettings("sim.trace", num_traces);

	sc_clock *driver_clk_sig = new sc_clock(
		"node_clk0", radsim_config.GetDoubleKnob("sim_driver_period"), SC_NS);

	add_system *system = new add_system("add_system", driver_clk_sig, radsim_design1); //AKB ADDED

	sc_clock *driver_clk_sig2 = new sc_clock(
		"node_clk0", radsim_config.GetDoubleKnob("sim_driver_period"), SC_NS); //AKB ADDED

	add_system *system2 = new add_system("add_system", driver_clk_sig2, radsim_design2); //AKB ADDED

	sc_start();

	delete system;
	delete system2; //AKB ADDED
	delete driver_clk_sig;
	delete driver_clk_sig2; //AKB ADDED
	delete radsim_design1; //AKB ADDED
	delete radsim_design2; //AKB ADDED
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
