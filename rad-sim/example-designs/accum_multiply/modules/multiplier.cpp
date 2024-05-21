#include <multiplier.hpp>

multiplier::multiplier(const sc_module_name &name, unsigned int ififo_depth, unsigned int ofifo_depth)
    : RADSimModule(name), input_data_temp(1), output_data_temp(1), rst("rst") { // data_temp(1), I assume it means to init it with size 1
  // Define key constants
  this->ififo_depth = ififo_depth;
  this->ofifo_depth = ofifo_depth;

  // Set to 0
  internal_register = 0;

  // Initialize FIFO modules
  char fifo_name[25];
  std::string fifo_name_str;
  fifo_name_str = "multiplier_ififo";
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

  fifo_name_str = "multiplier_ofifo";
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
  sensitive << rst << ofifo_almost_full_signal << ofifo_rdata_signal 
            << axis_multiplier_interface.tvalid
            << ififo_almost_full_signal
            << ififo_empty_signal 
            << ofifo_empty_signal << output_ready << clk;
  // Sequential logic and its clock/reset setup
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true); // Reset is active high

  // This function must be defined & called for any RAD-Sim module to register
  // its info for automatically connecting to the NoC
  this->RegisterModuleInfo();
}

multiplier::~multiplier() {}

void multiplier::Assign() {
  // Data that needs to be comb: 
  if (rst.read()) {
    // Enable signals
    ififo_ren_signal.write(0);
    ofifo_ren_signal.write(0);

    // Ready valid signals
    axis_multiplier_interface.tready.write(false); 
    output_valid.write(0);

    // Output data
    output.write(0);
  } else {
    // Input Signals
    axis_multiplier_interface.tready.write(!ififo_almost_full_signal.read()); // Ready when ififo is not full
    // ififo_wen_signal.write(axis_multiplier_interface.tready.read() && axis_multiplier_interface.tvalid.read()); // write data when both are ready and valid

    // IFIFO ren:
    ififo_ren_signal.write(!ofifo_almost_full_signal.read() && !ififo_empty_signal.read()); // Tell to read data

    // Output fifo's read enable determined by if ready and if itself is empty
    // In comb logic 
    ofifo_ren_signal.write(output_ready.read() && !ofifo_empty_signal.read());
    output_valid.write(!ofifo_empty_signal.read()); // Output is valid when not empty

    // Output value, in comb to prevent data not ready on a clock edge where all 2 signals are set
    if (output_ready.read() && !ofifo_empty_signal.read()) {
      data_vector<int16_t> tdata = ofifo_rdata_signal.read();
      // std::cout << "OFIFO reading: " << ofifo_rdata_signal.read() << endl;
      output.write(tdata[0]);
    }
  }
}

void multiplier::Tick() {
  // Reset logic
  // Clear FIFO content, set signals to not ready
    // FIFO is cleared already by their rst signal
  ofifo_wen_signal.write(0);
  ififo_wen_signal.write(0);
  ififo_wdata_signal.write(0);
  ofifo_wdata_signal.write(0);
  // Clear Registers -- actually not necessary because we only look at output when count is correct
  internal_register = 0;
  wait();

  std::string port_name = module_name + ".axis_multiplier_interface";

  // Always @ positive edge of the clock
  while (true) {
    // Process Input - use axis, similar code from mvm.cpp rx_input_interface
    // Here we can use wen because the tready is written via comb logic.
    if (axis_multiplier_interface.tready.read() && axis_multiplier_interface.tvalid.read()) {
      // mvm code converting axis bv to data_vector
      sc_bv<AXIS_MAX_DATAW> tdata = axis_multiplier_interface.tdata.read();
      for (unsigned int lane_id = 0; lane_id < 1; lane_id++) {
          input_data_temp[lane_id] =
              tdata.range((lane_id + 1) * 16 - 1, lane_id * 16)
                  .to_int();
      }
      // Store to ififo
      ififo_wdata_signal.write(input_data_temp);
      ififo_wen_signal.write(true);
      // std::cout << "     MULTIPLIER RECEIVING: " <<  input_data_temp << endl;
    } else { 
      ififo_wen_signal.write(false);
    }

    // Process read to registers, and store directly to ofifo
    // Here wen can be written because we use almost_full signal, which gives some buffer zone to prevent the "lag by 1" issue
    if (!ofifo_almost_full_signal.read() && !ififo_empty_signal.read()) {
      // If we have space to write values and we do have value to write
      ofifo_wen_signal.write(true);
      data_vector<int16_t> tdata = ififo_rdata_signal.read(); 
      output_data_temp[0] = internal_register * tdata[0];
      // std::cout << "MULTIPLIER WRITING TO OFIFO: " << output_data_temp[0] << endl;
      ofifo_wdata_signal.write(output_data_temp);
      internal_register = tdata[0]; // rdata_signal returns a data_vector of int16_t
    } else {
      ofifo_wen_signal.write(false);
    }
    
    wait();
  }
}

void multiplier::RegisterModuleInfo() {
  std::string port_name;
  _num_noc_axis_slave_ports = 0;
  _num_noc_axis_master_ports = 0;
  port_name = module_name + ".axis_multiplier_interface";
  RegisterAxisSlavePort(port_name, &axis_multiplier_interface, DATAW, 0);
}
