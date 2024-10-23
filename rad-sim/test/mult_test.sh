#!/bin/bash
test_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $test_path

cp -f ../example-designs/mult/config.yml ../config.yml

(cd ../; python config.py mult)

(cd ../build; make run)
