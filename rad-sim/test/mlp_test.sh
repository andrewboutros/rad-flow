#!/bin/bash
test_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $test_path

cp -f ../example-designs/mlp/config.yml ../uni_config.yml

(cd ../; python config.py mlp)

# python gen_testcase.py <num_layers> <input_size> {<hidden_dims>} {<mvms_per_layer>}
(cd ../example-designs/mlp/compiler; python gen_testcase.py 4 512 512 512 256 128 4 3 2 2)
(cd ../build; make run)
