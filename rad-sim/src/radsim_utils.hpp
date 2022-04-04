#pragma once

#include <systemc.h>
#include <flit.hpp>
#include <radsim_globals.hpp>
#include <booksim_config.hpp>

// Returns the current simulation cycle number given a certain clock period
int GetSimulationCycle(double period);

// Returns the current simulation cycle number given the NoC clock period
int GetSimTime();

// Calculates the number of hops between source and destination nodes in a 2D grid of certain dimension in case of
// minimal XY routing
int CalculateXYGridHops(unsigned int src, unsigned int dest, unsigned int grid_dimension);

int VCIDFromType(Flit::FlitType type, BookSimConfig* config);