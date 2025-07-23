#!/bin/bash
test_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $test_path

(cd ../; python config.py producer_consumer -config ../example-designs/producer_consumer/config_equal_freq_adaptor.yml)

(cd ../build; make run)

(cd ../; python config.py producer_consumer -config ../example-designs/producer_consumer/config_normal.yml)

(cd ../build; make run)