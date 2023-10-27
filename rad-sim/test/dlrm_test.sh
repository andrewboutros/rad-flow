#!/bin/bash
test_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $test_path

(cd ../; python config.py dlrm)

(cd ../example-designs/dlrm/compiler; python dlrm.py)
(cd ../build; make run)
