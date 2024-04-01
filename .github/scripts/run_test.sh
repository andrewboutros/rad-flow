#!/bin/bash

TEST_ERROR=0
if [ -z $TEST ]; then
    ./rad-sim/test/$TEST_SCRIPT || TEST_ERROR=$?
else
    ./rad-sim/test/$TEST_SCRIPT -t $TEST || TEST_ERROR=$?
fi
sed -i "s/\x1B\[[0-9;]\{1,\}[A-Za-z]/**/g" $GITHUB_STEP_SUMMARY
exit "$TEST_ERROR"