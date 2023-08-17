Quick Start Guide
=================

Installing C++ compiler and CMake
---------------------------------
Building RAD-Sim requires a C++ compiler and CMake (3.19 or higher), so make sure these are installed on your system or install them using the following command:

.. code-block:: bash

    $ sudo apt install build-essential cmake cmake-curses-gui


Installing SystemC
------------------

You have to also install SystemC libraries on your system following these steps:

1. Download the SystemC 2.3.4 (Core SystemC Language and Examples tar.gz) from this `link <https://www.accellera.org/downloads/standards/systemc>`_.

2. Unzip the tar archive in your directory of choice (I usually prefer to create a “tools” directory in my home for all the tools I am installing on my system):

    .. code-block:: bash
        
        $ tar -xzvf systemc-2.3.4.tar.gz systemc-2.3.4/


3. In the systemc-2.3.4 directory, build the SystemC libraries using CMake by executing the following steps:

    - Create a new build directory:

        .. code-block:: bash

            $ pwd
            <path_to_systemc>/systemc-2.3.4
            $ mkdir build
            $ cd build
            $ ccmake ..

    - Press ``c`` to configure the CMake build, then press ``t`` to toggle into the advanced configuration mode. Make sure to change ``CMAKE_CXX_STANDARD`` to ``11`` and ``DISABLE_COPYRIGHT_MESSAGE`` to ``ON``. You can change this entry by pressing enter to type in the new value or toggle between ON/OFF. You can also change any of the other parameters as needed for your system.
    - After changing all needed configuration parameters, press ``c`` again to configure and then ``g`` to generate the configuration file.
    - Build and install the SystemC libraries by executing the following commands:

        .. code-block:: bash

            $ make
            ....
            [100%] Linking CXX shared library libsystemc.so
            [100%] Built target systemc

            $ make check
            ....
            100% tests passed, 0 tests failed out of 33
            Total Test time (real) =   1.79 sec
            [100%] Built target check

            $ sudo make install

Installing Python Dependencies
------------------------------
It is preferable to install Anaconda and create a Python virtual environment for the RAD flow by following these steps:

1. `Download Anaconda <https://www.anaconda.com/products/distribution>`_ and install it following these `instructions <https://docs.anaconda.com/anaconda/install/linux/>`_
2. Create a new conda environment with all the required packages for the RAD flow using this command:

    .. code-block:: bash

        $ conda env create -f <rad_flow_root_dir>/rad-flow-env.yml
        Collecting package metadata (repodata.json): /
        ....
        #
        # To activate this environment, use
        #
        #     $ conda activate radflow
        #
        # To deactivate an active environment, use
        #
        #     $ conda deactivate

Building RAD-Sim
----------------

You can configure RAD-Sim for your example design simulation using the following commands executed at the ``rad-sim`` root directory (the commands use the ``mlp`` example design which can be replaced by your own design under the ``rad-flow/rad-sim/example-designs`` directory):

.. code-block:: bash

    $ cd <rad_flow_root_dir>/rad-sim
    $ python config.py mlp

Running RAD-Sim
----------------

To test that RAD-Sim is working as intended, the multi-layer perceptron (MLP) example design can be used as a test. You can simulate this example design following these steps:

1. Activate the RAD flow conda environment:

    .. code-block:: bash
        
        $ conda activate radflow

2. Generate an MLP test case using the provided compiler:

    .. code-block:: bash

        $ cd <rad_flow_root_dir>/rad-sim/example-designs/mlp/compiler
        # python gen_testcase.py <num_layers> <input_size> {<hidden_dims>} {<mvms_per_layer>}
        $ python gen_testcase.py 4 512 512 512 256 128 4 3 2 2
        # Number of layers = 4
        # Input dimension = 512
        # Hidden dimensions = [ 512 512 256 128 ]
        # Number of MVMs = [ 4 3 2 2 ]

3. Run RAD-Sim simulation:

    .. code-block:: bash

        $ cd <rad_flow_root_dir>/rad-sim/build
        $ make run
        # ....
        # SUCCESS - All outputs are matching!
        # Simulation Cycles = 11900
        # Aggregate NoC BW = 33.9383 Gbps

        # Info: /OSCI/SystemC: Simulation stopped by user.
        # [100%] Built target run