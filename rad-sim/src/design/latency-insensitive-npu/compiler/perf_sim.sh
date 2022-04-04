#!/bin/bash

cd ../../../..
#mkdir build
cd build
#cmake .. &> cmake.log
#make &> make.log
{ time make run_system; } &> ../sim.log
cp ../sim.log ../src/design/latency-insensitive-npu
#cd ..
#rm -rf build
