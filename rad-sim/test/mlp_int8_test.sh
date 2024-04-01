#!/bin/bash
test_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $test_path

(cd ../; python config.py mlp_int8)

# python gen_testcase.py <num_layers> <input_size> {<hidden_dims>} {<mvms_per_layer>}
(cd ../example-designs/mlp_int8/compiler; python gen_testcase.py 4 512 512 512 256 128 3 3 2 2)
(cd ../example-designs/mlp_int8/compiler; sed -Ei "s/([0-9]*),([0-9]*)/\2,\1/g" layer_mvm_config) # Changes all instances to verilator instances
(cd ../build; make run)
