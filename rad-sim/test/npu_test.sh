#!/bin/bash
test_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $test_path

(cd ../; python config.py npu)

(cd ../example-designs/npu/compiler; chmod 777 perf_sim.sh)
(cd ../example-designs/npu/scripts; python perf_tests.py)
