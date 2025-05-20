Running RAD-Gen
============================


A library of unit tests can be found at <rad_gen_top>/unit_tests. They include all the relevant configuration files needed for RAD-Gen modes of operation.
Execution of the unit tests is shown in the following code block:

.. code-block:: bash

   $ # from <rad_gen_top>
   $ python3 unit_tests/ci_tests.py
   $ # If a user wants to print (and not run) the relevant CLI commands which will be executed for each test they can add the "-p" or "--just_print" flag 
   $ python3 unit_tests/ci_tests.py -p

The following examples are taken from the ci_tests.py script.

ALU ASIC Flow Example
--------------------------------

The below example demonstrates how a user can sweep an ALU across a range of target clock frequencies specified in a configuration file and run a hammer based asic flow for one of the sweep data points. 

.. code-block:: bash

   $ # from <rad_gen_top>
   $ python3 rad_gen.py --subtools asic_dse --env_config_path unit_tests/inputs/asic_dse/sys_configs/asic_dse_env.yml --design_sweep_config unit_tests/inputs/asic_dse/sweeps/alu_sweep.yml

The above command will create a few different configuration files in the unit_tests/inputs/asic_dse/alu/configs directory each of which has a different target clock frequency.
We will then execute the asic flow with a single one of these configurations.

.. code-block:: bash

   $ # from <rad_gen_top>
   $ python3 rad_gen.py --subtools asic_dse --env_config_path unit_tests/inputs/asic_dse/sys_configs/asic_dse_env.yml --flow_mode hammer --top_lvl_module alu_ver --hdl_path unit_tests/inputs/asic_dse/alu/rtl --manual_obj_dir unit_tests/outputs/asic_dse/alu_ver/alu_ver_hammer_ci_test --flow_config_paths unit_tests/inputs/asic_dse/sys_configs/asap7.yml unit_tests/inputs/asic_dse/sys_configs/cadence_tools.yml unit_tests/inputs/asic_dse/alu/configs/alu_period_2.0.yaml 

At this point its useful to begin to breakdown some of the above cli arguments to better understand what the tool is doing.
More information about each one of these commands/configurations can be found in the :ref:`ASIC-DSE` section of the documentation.

At the end of the flow you should get an output report that looks something like below. This is a summary of the results of the flow.
It has sections for PPA, VLSI parameters, and hardware information. There are also sections for what stage of the asic flow the PPA results come from.
This gives users an idea of their accuracy.

.. code-block:: bash

   # --------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------
   #  Target Freq  |  Timing SRC  |    Slack     |    Delay     |Top Level Inst|  Total Area  |   Area SRC   |  Power SRC   | Total Power  |   GDS Area   
   # --------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------
   #     2.0 ns    |     par      |   1332.785   |   2016.213   |   alu_ver    |   1084.519   |     par      |     par      |  0.1590735   |  159.437394  
   # --------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------

COFFE FPGA Fabric w/ALU hardblock Flow Example
-------------------------------------------------------------

We will use the COFFE subtool in RAD-Gen to size a 7nm FPGA fabric with an ALU hardblock. 
COFFE will perform transistor sizing for fpga custom circuit logic and muxing required to interact with the ALU hardblock. 
The hardblock will be ran through a hammer based asic flow using ASAP7.

.. code-block:: bash

   $ python3 rad_gen.py --subtools coffe --max_iterations 1 --fpga_arch_conf_path unit_tests/inputs/coffe/finfet_7nm_fabric_w_hbs/finfet_7nm_fabric_w_hbs.yml --hb_flows_conf_path unit_tests/inputs/coffe/finfet_7nm_fabric_w_hbs/hb_flows.yml


IC 3D Flow Example
--------------------------------

The below example calls the IC_3D subtool, the flags determine if buffer DSE, PDN modeling, or other options are performed.

.. code-block:: bash

   $ python3 rad_gen.py --subtools ic_3d --input_config_path unit_tests/inputs/ic_3d/3D_ic_explore.yaml --buffer_dse
   $ # to run PDN modeling replace the --buffer_dse flag with the --pdn_modeling flag as shown below:
   $ python3 rad_gen.py --subtools ic_3d --input_config_path unit_tests/inputs/ic_3d/3D_ic_explore.yaml --pdn_modeling
