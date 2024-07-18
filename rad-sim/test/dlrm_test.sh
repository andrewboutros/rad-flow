#!/bin/bash
test_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $test_path

cp -f ../example-designs/dlrm/config.yml ../uni_config.yml

(cd ../; python config.py dlrm)

(cd ../example-designs/dlrm/compiler; python dlrm.py)
(cd ../build; make run)
