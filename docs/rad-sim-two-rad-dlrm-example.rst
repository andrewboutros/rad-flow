Two-RAD DLRM Example Design
=================

This guide explains how to use the two-RAD DLRM example design. RAD 1 is responsible for the DLRM up to and including the embedding table lookups.
These are then transmitted to RAD 2 over the inter-RAD network, which then completes the remaining model stages.

Building RAD-Sim
----------------

You can configure RAD-Sim for the two-RAD DLRM design simulation using the following commands executed at the ``rad-sim`` root directory.

.. code-block:: bash

    $ cd <rad_flow_root_dir>/rad-sim
    $ python config.py dlrm_two_rad #dlrm_two_rad is name of design directory within example-designs parent directory

Running RAD-Sim
----------------

You can then simulate this two-RAD DLRM example design following these steps:


1. Generate a DLRM test case using the provided compiler:

    .. code-block:: bash

        $ cd <rad_flow_root_dir>/rad-sim/example-designs/dlrm_two_rad/compiler
        $ python dlrm.py

2. Run RAD-Sim simulation:

    .. code-block:: bash

        $ cd <rad_flow_root_dir>/rad-sim/build
        $ make run
        # Info: /OSCI/SystemC: Simulation stopped by user.
        # Simulation Cycles from main.cpp = 20390
        # [100%] Built target run
        # dlrm_system.driver: Finished sending all inputs to embedding lookup module!
        # dlrm_system.dut.feature_interaction_inst: Got all memory responses at cycle 6113!
        # [==================================================] 100 %
        # Got 2048 output(s)!
        # Simulation PASSED! All outputs matching!
        # Simulated 19958 cycle(s)

        # Info: /OSCI/SystemC: Simulation stopped by user.
        # Simulation Cycles from main.cpp = 19971
        # [100%] Built target run