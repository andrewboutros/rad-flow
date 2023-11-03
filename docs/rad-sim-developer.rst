RAD-Sim Developers
===================

RAD-Sim Testing Infrastructure
-------------------------------

Python Scripts Tests
^^^^^^^^^^^^^^^^^^^^^
To run python tests, ensure the current working directory is in the ``rad-sim`` folder and run the following steps:

#. ``python -m unittest discover .``

If you would like a test report XML to be generated, run the following steps:

#. ``pip install -r test/requirements.txt``
#. ``python -m xmlrunner discover . --output-file test/python-test-report.xml``

MLP Tests
^^^^^^^^^^
To run the MLP test, ensure the current working directory is in the ``rad-sim`` folder and run the following steps:

#. ``conda activate radflow``
#. ``test/mlp_test.sh``

DLRM Tests
^^^^^^^^^^
To run the DLRM test, ensure the current working directory is in the ``rad-sim`` folder and run the following steps:

#. ``conda activate radflow``
#. ``test/dlrm_test.sh``

NPU Tests
^^^^^^^^^^
To run the NPU test, ensure the current working directory is in the ``rad-sim`` folder and run the following steps:

#. ``conda activate radflow``
#. ``test/npu_test.sh``

If you would like to run specific tests (ex. std_gemv, std_rnn, std_gru, std_lstm, std_mlp) it can be specified using the -t flag:

``test/npu_test.sh -t {test}``

NPU Tests rely on a baseline file located in the ``rad-sim/example-designs/npu/scripts/perf_baseline`` directory.
This file provides expected QoR and runtime values for each test case. The format of this file is ``{test} {QoR} {runtime}``.

Running the NPU test produces a correctness result (PASS/FAIL), along with a performance result (PASS/FAIL).
The performance result requires both the QoR value and the runtime to fall within specified thresholds from the baseline.
These thresholds are defined in the ``rad-sim/example-designs/npu/scripts/perf_tests.py`` file.