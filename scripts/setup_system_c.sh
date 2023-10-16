#!/bin/bash

sudo apt-get update && apt-get upgrade -y
sudo apt install build-essential cmake

# SYSTEMC

# Set variables for SystemC
SYSTEMC_VERSION="2.3.4"

wget -O systemc-$SYSTEMC_VERSION.tar.gz https://github.com/accellera-official/systemc/archive/refs/tags/$SYSTEMC_VERSION.tar.gz
tar xzf systemc-$SYSTEMC_VERSION.tar.gz
rm -f systemc-$SYSTEMC_VERSION.tar.gz
cd systemc-$SYSTEMC_VERSION
mkdir build
cd build
cmake ../ -DCMAKE_CXX_STANDARD=17
cmake --build .
