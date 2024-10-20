#!/bin/bash
test_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $test_path

cp -f ../example-designs/add/config.yml ../config.yml

(cd ../; python config.py add)

(cd ../build; make run)