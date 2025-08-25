Installation and Setup
=====================================


Run the following commands sequentially to install RAD-Gen and its dependencies.

Clone Repository
------------------

.. code-block:: bash

   $ cd ~ && git clone --recurse-submodules git@github.com:StephenMoreOSU/rad_gen.git
   $ cd rad_gen


Python Setup
------------------

.. code-block:: bash

   $ # create conda env from yaml file
   $ conda env create -f conda_env/env.yaml
   $ # activate conda env with below command
   $ conda activate rad-gen-env
   $ # For developers its convenient to install some libraries as editable python libs
   $ # We can do this in hammer with the following commands:
   $ cd vlsi/hammer
   $ # Installs an editable version of hammer to the conda env
   $ python3 -m pip install -e .
   $ # source a script which adds hammer modules to PYTHONPATH
   $ source env_setup.sh

ASAP7 PDK Setup
------------------

#. The ASAP7 pdk is large so its not specified as a submodule to RAD-Gen. Users will have to clone this themselves, create a directory in a desired workspace that will be used to store ASAP7 and possibly other pdks:
#. Optional: after cloning the ASAP7 follow `instructions <https://github.com/The-OpenROAD-Project/asap7/blob/master/asap7PDK_r1p7/README_ASAP7PDK_INSTALL_201210a.txt>`_ to set it up for cadence virtuoso
    a. This is only needed if virtuoso GDS extraction / DRC / LVS is required (not needed for vanilla asic flow)

Clone the ASAP7 repo in a directory of your choosing.

.. code-block:: bash

   $ cd ~ && git clone git@github.com:The-OpenROAD-Project/asap7.git

#. ASAP7 is supported for open source and commercial tools:
    * Commercial: Cadence
    * genus → Synthesis
    * innovus → Place & Route
    * tempus → Static Timing Analysis
    * Open Source: `OpenROAD <https://github.com/The-OpenROAD-Project/OpenROAD>`_

RAD-Gen ASIC Flow ASAP7 Specific Dependencies
----------------------------------------------

- `Cadence Genus <https://www.cadence.com/en_US/home/tools/digital-design-and-signoff/synthesis/genus-synthesis-solution.html>`_ → Synthesis
- `Cadence Innovus <https://www.cadence.com/en_US/home/tools/digital-design-and-signoff/soc-implementation-and-floorplanning/innovus-implementation-system.html>`_ → Place & Route
- `Synopsys PrimeTime <https://www.synopsys.com/implementation-and-signoff/signoff/primetime.html>`_ → Timing & Power
- `(Optional) Cadence Virtuoso <https://www.cadence.com/en_US/home/tools/custom-ic-analog-rf-design/layout-design/virtuoso-layout-suite.html>`_ → Full custom + GDS manipulation

The above commercial tools are required for the below examples, however, due to hammers support for OpenROAD, it would be possible to run it was well with modification to config files. However this is **untested**.


If they are installed correctly the following commands should return executable paths:

.. code-block:: bash

   $ which genus
   $ which innovus
   $ which pt_shell
   $ # Below for COFFE / 3D IC flow
   $ which hspice
   $ # Below is Optional
   $ which virtuoso


