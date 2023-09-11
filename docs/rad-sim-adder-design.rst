Example: Adder Design
================================
In this example, a simple design of a numeric adder is explored to illustrate the usage of different elements in RAD-Sim.

This guide serves as a complement to the source code provided in the ``rad-flow/rad-sim/example-designs/add`` directory of the repo.

Design Architecture
-------------------
This design consists of two modules: the client, and the adder. These modules communicate via the AXI-S protocol over a Network on Chip (NoC).
The client is responsible for taking numbers from the driver, and sending requests to the NoC.
The adder receives these requests via the NoC, and adds together all the numbers, producing a final result displayed to the user.

File Structure Overview
-----------------------------------

.. code-block::

        add
        ├── modules         
        │   ├── adder.cpp
        │   ├── adder.hpp
        │   ├── client.cpp
        │   └── client.hpp
        ├── add_driver.cpp         
        ├── add_driver.hpp
        ├── add_system.cpp       
        ├── add_system.hpp
        ├── add_top.cpp
        ├── add_top.hpp
        ├── add.clks
        ├── add.place
        ├── CMakeLists.txt
        └── config.yml

Prerequisites
--------------
It is recommended to be familiar with :doc:`rad-sim-code-structure` before following this example.

The source code for this example design can be found included in the RAD-Sim GitHub repository, under ``rad-flow/rad-sim/example-designs/add``.

Components
----------

modules/adder.{cpp/hpp}
^^^^^^^^^^^^^^^^^^^^^^^
The adder module is responsible for receiving numbers from the NoC, and keeping a running sum of the numbers receieved.
When the last number flag is receieved, the final result is displayed to the user.

In each rad-sim module, there are two main functions:

#. Assign: Reset function, sensitive to the ``rst`` signal. Equivalent to ``always @ (rst) begin ...``
#. Tick: Runs on every clock cycle. Equivalent to ``always @ (rst) begin ...``

On every clock cycle, the adder module should update the running sum if data is available.
The last number is flagged using ``t_finished``.

.. code-block:: C++

    if (axis_adder_interface.tvalid.read() && axis_adder_interface.tready.read()) {
      uint64_t current_sum = adder_rolling_sum.to_uint64();
      adder_rolling_sum = current_sum + axis_adder_interface.tdata.read().to_uint64();
      t_finished.write(axis_adder_interface.tlast.read());
    }

    if (t_finished.read()) {
        std::cout << "The final sum of all the addends in the transactions is: " << adder_rolling_sum.to_uint64() << std::endl;
        sc_stop();
    }

modules/client.{cpp/hpp}
^^^^^^^^^^^^^^^^^^^^^^^^
The client module is responsible for forwarding information provided by the driver onto the NoC through the Advanced eXtensible Interface Stream (AXI-S) protocol.

The information provided by the driver is read in sequentially, and is placed on a queue for forwarding over the NoC.
The last number is also kept track of through the use of a flag.

.. code-block:: C++

    if (client_ready.read() && client_valid.read()) {
      client_tdata_fifo.push(client_tdata);
      testbench_tlast = client_tlast.read();
    }
    client_fifo_full.write(client_tdata_fifo.size() >= client_fifo_depth);
    
Important pre-requisite information is also gathered for use by the AXI-S protocol.

.. code-block:: C++

    sc_bv<DATAW> tdata = client_tdata_fifo.front();
    std::string src_port_name = module_name + ".axis_client_interface";
    std::string dst_port_name = "adder_inst.axis_adder_interface";
    uint64_t dst_addr = radsim_design.GetPortDestinationID(dst_port_name);
    uint64_t src_addr = radsim_design.GetPortDestinationID(src_port_name);

A brief overview of AXI-S protocol and important signals:

The AXI-S protocol is a simple protocol used to transport uni-directional data. It contains 9 main signals.

* ``tvalid``: Boolean. Used by the producer to signify when ``tdata`` and ``tlast`` are valid. 
* ``tready``: Boolean. Used by the consumer to signify it is ready to receive data.
* ``tdata``: Bit Vector. Data to be transferred from the producer to the consumer.
* ``tstrb``: Bit Vector. Indicates which bytes of ``tdata`` are processed as a data byte or a position byte.
* ``tkeep``: Bit Vector. Indicates which bytes of ``tdata`` are processed or ignored.
* ``tlast``: Boolean. Flag to indicate the last data flit.
* ``tid``: Bit Vector. Identifies different streams of data.
* ``tdest``: Bit Vector. Indicates the address of the destination module on the NoC. 
* ``tuser``: Bit Vector. Indicates the address of the source module on the NoC.

Asserting the right signals to send data over the NoC...

.. code-block:: C++

    axis_client_interface.tdest.write(dst_addr);
    axis_client_interface.tid.write(0);
    axis_client_interface.tstrb.write(0);
    axis_client_interface.tkeep.write(0);
    axis_client_interface.tuser.write(src_addr);
    axis_client_interface.tlast.write(testbench_tlast && (client_tdata_fifo.size() == 1));
    axis_client_interface.tdata.write(tdata);
    axis_client_interface.tvalid.write(true);

add_system.{cpp/hpp}
^^^^^^^^^^^^^^^^^^^^^
The design system file instantiates two elements necessary for RAD-Sim: the driver, and the Device Under Test (DUT).

.. code-block:: C++

    driver_inst = new add_driver("driver");
    dut_inst = new add_top("dut");

Connections between the top-level and driver modules are also made.

.. code-block:: C++

    driver_inst->clk(*driver_clk_sig);
    driver_inst->rst(rst_sig);
    driver_inst->client_tdata(client_tdata_sig);
    driver_inst->client_tlast(client_tlast_sig);
    driver_inst->client_valid(client_valid_sig);
    driver_inst->client_ready(client_ready_sig);

    dut_inst->rst(rst_sig);
    dut_inst->client_tdata(client_tdata_sig);
    dut_inst->client_tlast(client_tlast_sig);
    dut_inst->client_valid(client_valid_sig);
    dut_inst->client_ready(client_ready_sig);

add_top.{cpp/hpp}
^^^^^^^^^^^^^^^^^^^^^
The design top-level file instantiates the two modules that are connected to the NoC (adder, client). This makes up the DUT.

.. code-block:: C++

    client_inst = new client(module_name, 16);
    adder_inst = new adder(module_name);

Non-NoC signal connections are also made to the wires connecting to the system design file (add_system).

.. code-block:: C++

    client_inst->rst(rst);
    client_inst->client_tdata(client_tdata);
    client_inst->client_tlast(client_tlast);
    client_inst->client_valid(client_valid);
    client_inst->client_ready(client_ready);

    adder_inst->rst(rst);

add_driver.{cpp/hpp}
^^^^^^^^^^^^^^^^^^^^^
The driver is the testbench for the entire design. For the add example, the testbench should generate N numbers and add it to a queue.

.. code-block:: C++

    // Random Seed
    srand (time(NULL));

    // Generate random numbers to be added together by the adder
    for (unsigned int i = 0; i < NUM_ADDENDS; i++) {
        unsigned int r_num = std::rand() % 10 + 1;
        std::cout << r_num << " ";
        numbers_to_send.push_back(r_num);
    }

The numbers in this queue are fed to the client module, as shown in the following flow chart.

.. image:: _static/add_driver_flowchart.png
  :width: 1000
  :alt: Add Driver Flowchart

A couple of signals are used to relay important information to the client.

* ``client_tdata``: Bit Vector that stores the number
* ``client_tlast``: Boolean flag to signify the last number being sent
* ``client_valid``: The number sent is ready to be read

A simple implementation of this can be done using the ``std::queue`` data structure.

.. code-block:: C++

    while (!numbers_to_send.empty()) {
        client_tdata.write(numbers_to_send.front());
        client_tlast.write(numbers_to_send.size() <= 1);
        client_valid.write(true);

        wait();

        if (client_valid.read() && client_ready.read()) {
        numbers_to_send.pop();
        }
    }

The client will read each of the numbers sent by the driver, and eventually forward the requests over the NoC.


add.clks
^^^^^^^^^
The same clock can be used between the two modules, running at the same frequency.

add.place
^^^^^^^^^
The two modules (adder, client) are connected to the same NoC (0), with ports 0 and 3 respectively.

CMakeLists.txt
^^^^^^^^^^^^^^
In addition to the standard items in the CMakeLists.txt file required for the build and linking, all source and header files to be built in this example design are added to the ``srcfiles`` and ``hdrfiles`` variables.

.. code-block::

        srcfiles:
            modules/adder.cpp
            modules/client.cpp
            add_top.cpp
            add_driver.cpp
            add_system.cpp

        hdrfiles:
            modules/adder.hpp
            modules/client.hpp
            add_top.hpp
            add_driver.hpp
            add_system.hpp

config.yml
^^^^^^^^^^
As this is a basic primitive design of RAD-Sim, the example config provided in :doc:`rad-sim-code-structure` is used.

Running the Example
-------------------
Running the example with ``NUM_ADDENDS = 3``, the following output is produced.

.. code-block:: text

    Generating Random Numbers to be added ...
    4 9 6 
    ----------------------------------------
    client_inst: Pushed request to FIFO
    client_inst: Pushed request to FIFO
    client_inst: Sent Transaction!
    Finished sending all numbers to client module!
    client_inst: Pushed request to FIFO
    client_inst: Sent Transaction!
    client_inst: Sent Transaction!
    adder_inst: Got Transaction (user = 3) (addend = 4)!
    adder_inst: Got Transaction (user = 3) (addend = 9)!
    adder_inst: Got Transaction (user = 3) (addend = 6)!
    adder_inst: The final sum of all the addends in the transactions is: 19