#!/bin/bash
usage()
{
   echo ""
   echo "Usage: $0 -t test"
   echo -e "\t-t The keyword on which NPU tests to run (default: run all)"
}

while getopts "t:h" opt
do
   case "$opt" in
      t) TEST_KEYWORD="$OPTARG" ;;
      h) usage
         exit 0
         ;;
      *) usage
         exit 1
         ;;
   esac
done

test_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $test_path

(cd ../; python config.py npu)

(cd ../example-designs/npu/compiler; chmod 777 perf_sim.sh)
if [ -z ${TEST_KEYWORD+x} ]; then
    (cd ../example-designs/npu/scripts; python perf_tests.py | tee -a $GITHUB_STEP_SUMMARY; exit ${PIPESTATUS[0]})
else
    (cd ../example-designs/npu/scripts; python perf_tests.py --run_test ${TEST_KEYWORD} | tee -a $GITHUB_STEP_SUMMARY; exit ${PIPESTATUS[0]})
fi
