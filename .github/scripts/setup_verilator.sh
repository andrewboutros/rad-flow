#!/bin/bash
sudo apt-get update && apt-get upgrade -y
sudo apt install git help2man perl python3 make autoconf g++ flex bison ccache
sudo apt install libgoogle-perftools-dev numactl perl-doc
sudo apt install libfl2
sudo apt install libfl-dev
sudo apt install zlibc zlib1g zlib1g-dev

git clone https://github.com/verilator/verilator
unset VERILATOR_ROOT  # For bash
cd verilator
autoconf         # Create ./configure script
./configure      # Configure and create Makefile
make -j `nproc`  # Build Verilator itself (if error, try just 'make')
sudo make install