#include <adder.hpp>

//TODO
/*
constructor: define depth and initialize counter to 0. Assign only sensitive on FIFO variables

assign: we probably don't need any assign signals, since we only write from this using axis. Maybe use this for fifo ren and wen

tick: 
  reset logic: 
    all signals to NOT READY (axis)
    clear all FIFO content, 
    reset count, 
    clear all registers
  check if input ready and input valid, push to ififo
  if counter is 4, and ofifo isn't full, push to ofifo.
  check if counter is NOT 4, and input NOT empty, pop from ififo (ren) and into registers (which are shifted), increment counter by 1
  axis interface write OFIFO content and ren ofifo. (to pop)

some specific coding considerations:
  ren for ififo should be in an if else statement, default ren to false. 
  if(count is right and i not empty){
    read from ififo input signal
    ren=true // this removes rdata on next clock edge, so next cycle if we should read it will be new data. 
  }else{
    ren=false
  }
*/

adder::adder(const sc_module_name &name, unsigned int ififo_depth, unsigned int ofifo_depth)
    : RADSimModule(name), input_data_temp(1), output_data_temp(1), rst("rst") { // data_temp(1), I assume it means to init it with size 1
  // Define key constants
  this->ififo_depth = ififo_depth;
  this->ofifo_depth = ofifo_depth;

  // Initialize value to 0
  for (int i = 0; i < NUMSUM; i++) {
    internal_registers[i] = 0;
  }
  num_values_received = 0;
  temp_sum = 0;

  // Initialize FIFO modules
  char fifo_name[25];
  std::string fifo_name_str;
  fifo_name_str = "adder_ififo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  ififo = new fifo<int16_t>(fifo_name, ififo_depth, 16, ofifo_depth-1, 0); // width is 16 for int16, almost_full is 1 less
  ififo->clk(clk);
  ififo->rst(rst);
  ififo->wen(ififo_wen_signal);
  ififo->ren(ififo_ren_signal);
  ififo->wdata(ififo_wdata_signal);
  ififo->full(ififo_full_signal);
  ififo->almost_full(ififo_almost_full_signal);
  ififo->empty(ififo_empty_signal);
  ififo->almost_empty(ififo_almost_empty_signal);
  ififo->rdata(ififo_rdata_signal);

  fifo_name_str = "adder_ofifo";
  std::strcpy(fifo_name, fifo_name_str.c_str());
  ofifo = new fifo<int16_t>(fifo_name, ofifo_depth, 16, ofifo_depth-1, 0); // width is 16 for int16, almost_full is 1 less
  ofifo->clk(clk);
  ofifo->rst(rst);
  ofifo->wen(ofifo_wen_signal);
  ofifo->ren(ofifo_ren_signal);
  ofifo->wdata(ofifo_wdata_signal);
  ofifo->full(ofifo_full_signal);
  ofifo->almost_full(ofifo_almost_full_signal);
  ofifo->empty(ofifo_empty_signal);
  ofifo->almost_empty(ofifo_almost_empty_signal);
  ofifo->rdata(ofifo_rdata_signal);

  // Combinational logic and its sensitivity list
  SC_METHOD(Assign);
  sensitive << rst << ofifo_almost_full_signal << ofifo_rdata_signal 
            << axis_adder_interface.tready
            << ififo_almost_full_signal
            << ififo_empty_signal 
            << ofifo_empty_signal << input << input_valid;
  
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

adder::~adder() {}

void adder::Assign() {
  if (rst.read()) {
    // Set axis signals to default not ready, following tx_input_interface in mvm.cpp
    axis_adder_interface.tdata.write(0); // No data written
    axis_adder_interface.tvalid.write(false); // Data isn't being written
    axis_adder_interface.tstrb.write(0b11); // Strobe (we transfer 16 bit int)
    axis_adder_interface.tkeep.write(0b11); // Keep (all bytes are valid)
    axis_adder_interface.tlast.write(0); // We aren't sending data, default is 0
    axis_adder_interface.tuser.write(0); // User defined signal, default 0
    
    // FIFO is controlled non-sequentially
    ififo_wen_signal.write(0);
    ififo_ren_signal.write(0);
    ofifo_wen_signal.write(0);
    ofifo_ren_signal.write(0);

    // Reset input signals
    input_ready.write(0);

    // FIFO wdata done in comb logic, always equal to input data
    ififo_wdata_signal.write(0);
    ofifo_wdata_signal.write(0);
  } else {
    // Process Input
    input_data_temp[0] = input.read(); // Convert data type to a data_vector<>
    // std:: cout << "input_data_temp[0]: " << (input_data_temp[0]) << endl;
    ififo_wdata_signal.write(input_data_temp); // Data is input
    input_ready.write(!ififo_almost_full_signal.read()); // input_ready only depend on ififo full or not
    ififo_wen_signal.write(!ififo_almost_full_signal.read() && input_valid.read()); // ififo wen always true when both signals ready

    // IFIFO to registers
    // we read whenever it's ready (ififo isn't empty and we have space in registers)
    ififo_ren_signal.write(num_values_received < NUMSUM && !ififo_empty_signal.read());
    std::cout << "ififo empty signal read by adder: " << ififo_empty_signal.read() << " ififo pop en: " << (num_values_received < NUMSUM && !ififo_empty_signal.read()) << " num val rec: " << num_values_received << endl;

    // Registers to OFIFO
    temp_sum = 0;
    for (int i = 0; i < NUMSUM; i++){
      temp_sum += internal_registers[i];
    }
    output_data_temp[0] = temp_sum; 
    ofifo_wdata_signal.write(output_data_temp);
    ofifo_wen_signal.write(num_values_received == NUMSUM && !ofifo_almost_full_signal.read());
    // std::cout << "num_values_rec == numsum: " << (num_values_received == NUMSUM) << " num values: " << num_values_received << endl;

    // OFIFO output
    data_vector<int16_t> tdata = ofifo_rdata_signal.read();
    if (tdata.size() > 0 && !ofifo_empty_signal.read()) {
      // AXIS code copied from mvm, assume LANES=1 for 1 single value here, bitwitdh=16 for int16_t
      // Problem that might occur: in mvm this was in assign block
      std::string dest_name;
      unsigned int dest_id;
      dest_name = "multiplier_inst.axis_multiplier_interface";
      dest_id = radsim_design.GetPortDestinationID(dest_name);
      sc_bv<AXIS_MAX_DATAW> axis_adder_interface_tdata_bv;
      for (unsigned int lane_id = 0; lane_id < 1; lane_id++) {
        // std::cout << "lane id " << lane_id << endl;
        // std::cout << "ofifo empty" << ofifo_empty_signal.read() << endl;
        // std::cout << "ofifo empty" << ofifo_empty_signal << endl;
        // std::cout << tdata.size() << endl;
        axis_adder_interface_tdata_bv.range((lane_id + 1) * 16 - 1, lane_id * 16) =
            tdata[lane_id];
      }
      axis_adder_interface.tdata.write(axis_adder_interface_tdata_bv);
      axis_adder_interface.tvalid.write(true);
      axis_adder_interface.tdest.write(dest_id);
    } else {
      axis_adder_interface.tvalid.write(false);
    }
    ofifo_ren_signal.write(axis_adder_interface.tvalid.read() && axis_adder_interface.tready.read());
  }

}

void adder::Tick() {
  // Reset logic
  // Reset Count
  num_values_received = 0;
  wait();

  std::string port_name = module_name + ".axis_adder_interface";

  // Always @ positive edge of the clock
  while (true) {
    /*
      check if input ready and input valid, push to ififo
      if counter is 4, and ofifo isn't full, push to ofifo.
      check if counter is NOT 4, and input NOT empty, pop from ififo (ren) and into registers (which are shifted), increment counter by 1
      axis interface write OFIFO content and ren ofifo. (to pop)
    */

    // Process compute to OFIFO
    // This is before updating num_values_received so value == numsum is possible
    if (num_values_received == NUMSUM && !ofifo_almost_full_signal.read()) {
      num_values_received = 0;
    }

    // Process read to registers
    if (num_values_received < NUMSUM && !ififo_empty_signal.read()) {
      // If we have space to write values and we do have value to write
      for (int i = NUMSUM-1; i > 0; i--) {
        internal_registers[i] = internal_registers[i-1]; // Shift all value by 1
      }
      data_vector<int16_t> tdata = ififo_rdata_signal.read(); 
      internal_registers[0] = tdata[0]; // rdata_signal returns a data_vector of int16_t
      num_values_received++; // New value received, increment
      // std::cout << "received " << num_values_received << " value: " << internal_registers[0] << "signal" << ififo_rdata_signal.read() << endl;
      // if (num_values_received > 2)
      // exit(1);
    }
    wait();
  }
}

void adder::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;

  port_name = module_name + ".axis_adder_interface";
  RegisterAxisMasterPort(port_name, &axis_adder_interface, DATAW, 0);
}
