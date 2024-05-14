#include <multiplier.hpp>

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

multiplier::multiplier(const sc_module_name &name, unsigned int ififo_depth, unsigned int ofifo_depth)
    : RADSimModule(name), input_data_temp(1), output_data_temp(1), rst("rst") { // data_temp(1), I assume it means to init it with size 1
  // Define key constants
  this->ififo_depth = ififo_depth;
  this->ofifo_depth = ofifo_depth;

  // Initialize FIFO modules
  char fifo_name[25];
  std::string fifo_name_str;
  fifo_name_str = "multiplier" + std::to_string(mvm_id) + "_ififo";
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

  fifo_name_str = "multiplier" + std::to_string(mvm_id) + "_ofifo";
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

  // Combinational logic and its sensitivity list TODO
  SC_METHOD(Assign);
  sensitive << rst << req_fifo_full;
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

multiplier::~multiplier() {}

void multiplier::Assign() {
}

void multiplier::Tick() {
  // Reset logic
  // Set axis signals to default not ready, following tx_input_interface in mvm.cpp
  axis_multiplier_interface.tdata.write(0); // No data written
  axis_multiplier_interface.tvalid.write(false); // Data isn't being written
  axis_multiplier_interface.tstrb.write(0b11); // Strobe (we transfer 16 bit int)
  axis_multiplier_interface.tkeep.write(0b11); // Keep (all bytes are valid)
  axis_multiplier_interface.tlast.write(0); // We aren't sending data, default is 0
  axis_multiplier_interface.tuser.write(0); // User defined signal, default 0
  // Clear FIFO content, set signals to not ready
    // FIFO is cleared already by their rst signal
  ififo_wen_signal.write(0);
  ififo_ren_signal.write(0);
  ififo_wdata_signal.write(0);
  ofifo_wen_signal.write(0);
  ofifo_ren_signal.write(0);
  ofifo_wdata_signal.write(0);
  // Clear Registers -- actually not necessary because we only look at output when count is correct
  for (int i = 0; i < NUMSUM; i++) {
    internal_registers[i] = 0;
  }
  // Reset Count
  num_values_received = 0;
  // Reset input signals
  input_ready.write(0);
  wait();

  std::string port_name = module_name + ".axis_multiplier_interface";

  // Always @ positive edge of the clock
  while (true) {
    /*
      check if input ready and input valid, push to ififo
      if counter is 4, and ofifo isn't full, push to ofifo.
      check if counter is NOT 4, and input NOT empty, pop from ififo (ren) and into registers (which are shifted), increment counter by 1
      axis interface write OFIFO content and ren ofifo. (to pop)
    */

    // Process Input
    if (input_ready.read() && input_valid.read()) {
      // When testbench can send input
      ififo_wen_signal.write(true); // Write to IFIFO
      input_data_temp[0] = input.read(); // Convert data type to a data_vector<>
      ififo_wdata_signal.write(input_data_temp); // Data is input
    } else {
      ififo_wen_signal.write(false); // Else, don't read
    }

    // Process read to registers
    if (num_values_received < NUMSUM && !ififo_empty_signal.read()) {
      // If we have space to write values and we do have value to write
      ififo_ren_signal.write(true); // Tell to read data
      for (int i = NUMSUM-1; i > 0; i--) {
        internal_registers[i] = internal_registers[i-1]; // Shift all value by 1
      }
      internal_registers[0] = ififo_rdata_signal.read()[0]; // rdata_signal returns a data_vector of int16_t
      num_values_received++; // New value received, increment
    } else {
      ififo_ren_signal.write(false); // Else don't pop any values
    }

    // Process compute to OFIFO
    if (num_values_received == NUMSUM && !ofifo_almost_full_signal.read()) {
      ofifo_wen_signal.write(true);
      temp_sum = 0;
      for (int i = 0; i < NUMSUM; i++){
        temp_sum += internal_registers[i];
      }
      output_data_temp[0] = temp_sum; 
      ofifo_wdata_signal.write(output_data_temp);
    } else {
      ofifo_wen_signal.write(false);
    }

    // Process output from OFIFO
    if (!ofifo_empty_signal.read()) {
      // Always attempt to output values
      // AXIS code copied from mvm, assume LANES=1 for 1 single value here, bitwitdh=16 for int16_t
      // Problem that might occur: in mvm this was in assign block
      std::string dest_name;
      unsigned int dest_id;
      dest_name = "multiplier_inst.axis_multiplier_interface";
      dest_id = radsim_design.GetPortDestinationID(dest_name);
      data_vector<int16_t> tx_tdata = ofifo_rdata_signal.read();
      sc_bv<AXIS_MAX_DATAW> axis_multiplier_interface_tdata_bv;
      for (unsigned int lane_id = 0; lane_id < 1; lane_id++) {
        axis_multiplier_interface_tdata_bv.range((lane_id + 1) * 16 - 1, lane_id * 16) =
            tx_tdata[lane_id];
      }
      axis_multiplier_interface.tdata.write(tx_tdata_bv);
      axis_multiplier_interface.tvalid.write(true);
      axis_multiplier_interface.tuser.write(dest_interface); // Need to confirm how to modify interface and id
      axis_multiplier_interface.tdest.write(dest_id);
      axis_multiplier_interface.tid.write(dest_interface_id);
    } else {
      // Don't write
      axis_multiplier_interface.tvalid.write(false);
    }
    // Pop ofifo content when both valid and ready for interface
    ofifo_ren_signal.write(axis_multiplier_interface.tvalid.read() && axis_multiplier_interface.tready.read());
    wait();
  }
}

void addder::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;

  port_name = module_name + ".axis_multiplier_interface";
  RegisterAxisMasterPort(port_name, &axis_multiplier_interface, DATAW, 0);
}
