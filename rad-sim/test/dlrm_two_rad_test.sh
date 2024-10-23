#!/bin/bash
test_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $test_path

cp -f ../example-designs/dlrm_two_rad/config.yml ../config.yml

(cd ../; python config.py dlrm_two_rad)

(cd ../example-designs/dlrm_two_rad/compiler; python dlrm.py)
(cd ../build; make run)
