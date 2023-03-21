#!/bin/sh

echo '16 LANES SOFT SMALL'
python dlrm.py -l 16 -n 256 -m ab_small.csv 
cd ../../../build
make >> make.log
./sim/build/system
cd ../example-designs/dlrm/compiler
echo '16 LANES HARD SMALL'
python dlrm.py -l 16 -n 256 -m ab_small.csv -a
cd ../../../build
make >> make.log
./sim/build/system
cd ../example-designs/dlrm/compiler
echo '32 LANES SOFT SMALL'
python dlrm.py -l 32 -n 256 -m ab_small.csv 
cd ../../../build
make >> make.log
./sim/build/system
cd ../example-designs/dlrm/compiler
echo '32 LANES HARD SMALL'
python dlrm.py -l 32 -n 256 -m ab_small.csv -a
cd ../../../build
make >> make.log
./sim/build/system
cd ../example-designs/dlrm/compiler
echo '64 LANES SOFT SMALL'
python dlrm.py -l 64 -n 256 -m ab_small.csv 
cd ../../../build
make >> make.log
./sim/build/system
cd ../example-designs/dlrm/compiler
echo '64 LANES HARD SMALL'
python dlrm.py -l 64 -n 256 -m ab_small.csv -a
cd ../../../build
make >> make.log
./sim/build/system
cd ../example-designs/dlrm/compiler