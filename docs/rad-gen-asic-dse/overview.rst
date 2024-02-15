ASIC-DSE Overview
----------------------------------------------------------

Command Line Options:

+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| CLI Option          | Description                                                                                            | Data Type | Choices                               |
+=====================+========================================================================================================+===========+=======================================+
| env_config_path     | Path to hammer environment configuration file                                                          | str       |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| design_sweep_config | Path to design sweep config file                                                                       | str       |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| run_mode            | Specify if flow is run in "serial" or "parallel" or "gen_scripts"                                      | str       | ["parallel", "serial", "gen_scripts"] |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| flow_mode           | Mode in which asic flow is run "hammer" or "custom" modes                                              | str       | ["custom" ,"hammer"]                  |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| top_lvl_module      | Top level module of design                                                                             | str       |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| hdl_path            | Path to directory containing hdl files                                                                 | str       |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| flow_config_paths   | Paths to flow config files, these can be either "custom" or "hammer" format                            | List[str] |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| use_latest_obj_dir  | Uses latest obj / work dir found in the respective output_design_files/<top_module> dir                | bool      |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| manual_obj_dir      | Uses user specified obj / work dir                                                                     | str       |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| compile_results     | Flag to compile results related a specific asic flow or sweep depending on additional provided configs | bool      |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| synthesis           | Flag to run synthesis                                                                                  | bool      |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| place_n_route       | Flag to run place & route                                                                              | bool      |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| primetime           | Flag to run primetime (timing & power)                                                                 | bool      |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| sram_compiler       | Flag that must be provided if sram macros exist in design                                              | bool      |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+
| make_build          | UNDER DEV Generates a makefile to manage flow dependencies and execution                               | bool      |                                       |
+---------------------+--------------------------------------------------------------------------------------------------------+-----------+---------------------------------------+



Modes of Operation
------------------

##################
ASIC Flow
##################

The ASIC flow has implementations using custom tcl scripts with either industry standard tools OR `Hammer <https://hammer-vlsi.readthedocs.io/en/stable/>`_, which is a tool that supports PDK and tool agnosic ASIC flows.
Users can control the mode of execution with the ``flow_mode`` CLI option.

There are a few different configuration files that are required to run the ASIC flow shown below.

+++++++++++++++++++++++++++++
ASIC-DSE Environment Config
+++++++++++++++++++++++++++++


Associated with the ``env_config_path`` cli option. 
This file that contains information thats relevant to all possible modes of operation of RAD-Gen, such as input/output paths and other enviornment variables.
Valid configuration parameters can be found in rad_gen/src/common/data_structs.py

Example asic_dse_env.yaml file
( <rad_gen_top>/unit_tests/inputs/asic_dse/sys_configs/asic_dse_env.yml):

.. code-block:: yaml

    env:
        rad_gen_home_path: ~/rad_gen
        # Path to hammer github repo
        hammer_home_path: ~/rad_gen/vlsi/hammer
        # CAD tool related env vars (hammer format)
        env_paths: [ ~/rad_gen/unit_tests/inputs/asic_dse/sys_configs/env.yaml ]
        design_input_path: ~/rad_gen/unit_tests/inputs/asic_dse
        design_output_path: ~/rad_gen/unit_tests/outputs/asic_dse


In the above example the possibly confusing "env_paths" field is used to specify the environment config file needed for Hammer.

**Hammer env.yaml file:**

This file is used to find the executables and licenses associated with industry ASIC tools. 

Example env.yaml file ( <rad_gen_top>/unit_tests/inputs/asic_dse/env.yaml ):

.. code-block:: yaml
    
    # Base path to where Mentor tools are installed
    mentor.mentor_home: "<your path to mentor home>/mentor"
    # Mentor license server/file
    synopsys.MGLS_LICENSE_FILE: ""
    # Base path to where Cadence tools are installed
    cadence.cadence_home: "<your path to cadence home>/cadence"
    # Cadence license server/file
    cadence.CDS_LIC_FILE: ""
    # Base path to where Synopsys tools are installed
    synopsys.synopsys_home: ""
    # Synopsys license server/files
    synopsys.SNPSLMD_LICENSE_FILE: ""


++++++++++++++++++++
Flow Configs
++++++++++++++++++++

Flow configs are the files that specify the PDK, ASIC tool, and design specific information. They differ depending on the `flow_mode` specified.

^^^^^^^^^^^^^^^^^^^^
Hammer
^^^^^^^^^^^^^^^^^^^^

Hammer flow files are in the Hammer IR format specfied `here <https://hammer-vlsi.readthedocs.io/en/1.1.2/Hammer-Use/Hammer-Config.html>`_.
Its good practice to seperate configuration files into PDK, tool, and design specific files. This allows for users to run the same design with different PDKs or Tools with ease.

Examples of these files can be found at <rad_gen_top>/unit_tests/inputs/asic_dse/sys_configs


**ASAP7 PDK Config asap7.yml**

.. code-block:: yaml

    technology.asap7:
        tarball_dir: ~/asap7

    vlsi.core:
        synthesis_tool_path: [~/rad_gen/vlsi/hammer/hammer/synthesis]
        synthesis_tool: genus
        
        par_tool_path: [~/rad_gen/vlsi/hammer/hammer/par]
        par_tool: innovus

        sram_generator_tool: "sram_compiler"
        sram_generator_tool_path: [~/rad_gen/vlsi/hammer/hammer/technology/asap7]
        sram_generator_tool_path_meta: "append"
        technology: asap7


**Cadence Tools Config cadence_tools.yml**

.. code-block:: yaml

    # Default settings for synthesis in Genus, for project/technology configuration and overriding.
    synthesis.genus:
        # Location of the binary.
        genus_bin: "/CMC/tools/cadence/GENUS20.10.000_lnx86/tools.lnx86/bin/genus"

        # Genus version to use.
        # Used to locate the binary - e.g. the '171' in ${cadence.cadence_home}/GENUS/GENUS171/bin/genus
        version: "201"

        # Generate the TCL file but do not run it yet.
        generate_only: false

    par.innovus:
        # Location of the binary.
        innovus_bin: "/CMC/tools/cadence/INNOVUS21.12.000_lnx86/tools.lnx86/bin/innovus"

        # Innovus version to use.
        # Used to locate the binary - e.g. the '171' in ${cadence.cadence_home}/INNOVUS/INNOVUS171/bin/innovus
        # 171_ISR3 supports ILMs properly in contrast to 171.
        version: "211"
    ...


One can see fields in the above configs which provide information about where to look for asic tool binaries and PDK information.



**Design Specific Config alu.yml**


.. code-block:: yaml

    # synthesis settings
    synthesis:
        #### Inputs between these comments are changed with -v and -t arguments of rad-gen, if they don't look correct in this file pass in those respective arguments ####
        inputs.input_files: [ 
            ~/rad_gen/unit_tests/inputs/asic_dse/alu/rtl/alu.sv,
            ~/rad_gen/unit_tests/inputs/asic_dse/alu/rtl/dff.sv
        ]
        inputs.top_module: alu_ver
        inputs.hdl_search_paths: [ ~/rad_gen/unit_tests/inputs/asic_dse/alu/rtl ]
        #### Inputs between these comments are changed with -v and -t arguments of rad-gen, if they don't look correct in this file pass in those respective arguments ####
        clock_gating_mode: "empty"
  
    # place and route settings
    par.innovus:
        floorplan_mode: "manual"
        floorplan_script_contents: "create_floorplan -core_margins_by die -flip f -die_size_by_io_height max -site asap7sc7p5t -stdcell_density_size {1.0 0.7 10 10 10 10}"
        design_flow_effort: "express"

    vlsi.inputs:
        power_spec_mode: "auto"
        power_spec_type: "cpf"
        clocks : [
            {
            name: "clk",
            period: "2 ns",
            }
        ]
    ...

In the design specific configuration file more specific information can be provided such as HDL sources, VLSI parameters, etc.

^^^^^^^^^^^^^^^^^^^^
Custom TCL Flow
^^^^^^^^^^^^^^^^^^^^

The custom flow is run by specifying the ``flow_mode`` CLI option as ``custom``.

The alternative option for using the ASIC flow is comprised of tcl scripts for a specific set of commercial ASIC tools. 
Specifically the tools used are Cadence Genus, Innovus, and Synopsys PrimeTime.

This flow is an artifact of previous work, however, it can be useful when one wants to use specific commands or flow options that aren't natively supported by Hammer.
There are also related parsing and plotting scripts associated with this custom flow.

Note: Hammer has an option to modify their ASIC flow with additional python APIs, however, for the unfamiliar user these can sometimes be challenging to use.

An example custom flow config can be found at <rad_gen_top>/unit_tests/inputs/asic_dse/alu/configs/alu_custom_flow.yml

The parameters for the custom flow are shown below:

+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| Parameter                            | Description                                                                                                                                                                                                                                            |            |
+======================================+========================================================================================================================================================================================================================================================+============+
| condensed_results_folder             | Path to directory which will contain PPA results for each stage of ASIC flow for hardblock across all sweep of all input params                                                                                                                        |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| parallel_hardblock_folder            | Path to dir containing all intermediate files for ASIC tools when running the parallel hardblock flow                                                                                                                                                  |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| mp_num_cores                         | maximum number of CPU cores allocated to parallel flow                                                                                                                                                                                                 |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| partition_flag                       | If you wish to use the hierarchical flow {True OR False}                                                                                                                                                                                               |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| ungroup_regex                        | This is a regular expression which matches to the names of all modules which you would like to keep grouped in the synthesis flow, it is required that all modules you wish to have in seperate partitions in place and route are matched by the regex |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| design_folder                        | path to directory containing all HDL design files (directory is searched recursively for all files with “design_language” extension                                                                                                                    |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| design_language                      | The design language. {verilog OR  vhdl OR sverilog}                                                                                                                                                                                                    |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| clock_pin_name                       | Name of the clock pin in the design                                                                                                                                                                                                                    |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| top_level                            | Name of the top-level entity in the design                                                                                                                                                                                                             |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| process_lib_paths                    | List of absolute paths to directories containing power_timing_noise information for the desired process                                                                                                                                                |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| target_libraries                     | List of absolute paths to “.db” libraries used for power_timing_noise information                                                                                                                                                                      |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| clock_period*                        | List of clock periods (ns)                                                                                                                                                                                                                             |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| wire_selection*                      | List of wire loading models to be used                                                                                                                                                                                                                 |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| static_probability                   | The percentage of time circuit signals are in “1” state, used for power analysis                                                                                                                                                                       |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| toggle_rate                          | The rate of 0→1 and 1→ 0 transistions in a period of time, used for power analysis                                                                                                                                                                     |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| synth_folder                         | Folder in which synthesis reports and outputs are stored in serial flow                                                                                                                                                                                |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| show_warnings                        | Show warnings during synthesis {True OR False}                                                                                                                                                                                                         |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| lef_files                            | List of absolute paths to all .lef files used in design                                                                                                                                                                                                |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| best_case_libs                       | List of absolute paths to all .lib files corresponding to standard cells used in design for best case corner                                                                                                                                           |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| standard_libs                        | List of absolute paths to all .lib files corresponding to standard cells used in design for typical corner                                                                                                                                             |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| worst_case_libs                      | List of absolute paths to all .lib files corresponding to standard cells used in design for worst case corner                                                                                                                                          |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| metal_layers*                        | List of highest metal layer allowed to be used in place and route (must be greater than length of metal_layer_names list)                                                                                                                              |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| core_utilization*                    | List of utilization of core site                                                                                                                                                                                                                       |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| tilehi_tielo_cells_between_power_gnd | enable tie-high tie-low cells {True OR False}                                                                                                                                                                                                          |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| core_site_name                       | name of core site in floorplan, can be found in .lef files                                                                                                                                                                                             |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| power_ring_width                     | Width of VDD/VSS power ring around the core of the design (um)                                                                                                                                                                                         |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| power_ring_spacing                   | Amount of spacing added b/w power ring and core (um)                                                                                                                                                                                                   |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| space_around_core                    | Amount of space the floorplan allocates around the core (Should be larger than 2 x power_ring_width + power_ring_spacing)                                                                                                                              |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| height_to_width_ratio                | ratio of height to width of the floorplan, 1.0 indicates a square and 4.0 would indicate a rectangle who’s height is 4x its width                                                                                                                      |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| metal_layer_names                    | List of names of metal layers starting from the bottom-most layer on the left                                                                                                                                                                          |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| power_ring_metal_layer_names         | List of names of metal layers to use for each side of the power ring      order: top, bottom, left, right                                                                                                                                              |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| filler_cell_names                    | List of filler cell names                                                                                                                                                                                                                              |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| gnd_net                              | Name of gnd_net found in library                                                                                                                                                                                                                       |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| gnd_pin                              | Name of gnd_pin found in library                                                                                                                                                                                                                       |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| pwr_net                              | Name of pwr_net found in library                                                                                                                                                                                                                       |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| pwr_pin                              | Name of pwr_pin found in library                                                                                                                                                                                                                       |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| inv_footprint                        | Name of inverter cell, can be found in .lef files (optional)                                                                                                                                                                                           |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| buf_footprint                        | Name of buffer cell, can be found in .lef files (optional)                                                                                                                                                                                             |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| delay_footprint                      | Name of delay  cell, can be found in .lef files (optional)                                                                                                                                                                                             |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| pnr_tool                             | Choice of Cadence based pnr tool {innovus                                                                                                                                                                                                              | encounter} |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| pr_folder                            | Folder in which place and route reports and outputs are stored in serial flow                                                                                                                                                                          |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| map_file                             | Name of the file to use for layer mapping. Used for stream out. {<stream_out_fname OR None }                                                                                                                                                           |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| primetime_libs                       | list of “.db” libraries names used for timing/power analysis                                                                                                                                                                                           |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+
| primetime_folder                     | Folder in which power and timing analysis reports and outputs are stored in serial flow                                                                                                                                                                |            |
+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+------------+






























