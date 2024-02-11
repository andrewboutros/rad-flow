#!/bin/bash

is_first=$1

if [ $is_first -eq 1 ] 
then
  cd ../../../
  if [ ! -d build ] 
  then
    mkdir build
  fi
  cd build
  cmake .. &> cmake.log
  make &> make.log
else
  cd ../../../build
fi
{ time make run; } &> sim.log
cp sim.log ../example-designs/npu/
cd ../example-designs/npu/compiler