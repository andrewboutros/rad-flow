#include <dram_test.hpp>

dram_test::dram_test(const sc_module_name& name) : sc_module(name) {
  SC_METHOD(assign);
  sensitive << rst;
  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

dram_test::~dram_test() {

}

void dram_test::source() {
  // Reset
  rst.write(true);
  wait();
  rst.write(false);
  wait();

  int num_write_commands = NUM_WRITE_CMDS;
  int num_read_commands = NUM_READ_CMDS;
  int i = 0;
  int addr = 0;
  int data = 32;
  sc_bv<AXIS_USERW> tuser;
  while (i < num_write_commands) {
    tuser.range(63, 0) = addr + i;
    tuser.range(64, 64) = 1;
    tx_interface.tdata.write(data + i);
    tx_interface.tuser.write(tuser);
    tx_interface.tvalid.write(true);
    wait();
    if (tx_interface.tvalid.read() && tx_interface.tready.read()) {
      i++;
    }
  }
  std::cout << "Finished sending all write commands!" << std::endl;
  tx_interface.tvalid.write(false);
  wait();

  i = 0;
  while (i < num_read_commands) {
    tuser.range(63, 0) = addr + i;
    tuser.range(64, 64) = 0;
    tx_interface.tdata.write(data + i);
    tx_interface.tuser.write(tuser);
    tx_interface.tvalid.write(true);
    wait();
    if (tx_interface.tvalid.read() && tx_interface.tready.read()) {
      i++;
    }
    std::cout << "Finished sending all read commands!" << std::endl;
    tx_interface.tvalid.write(false);
    wait();
  }
}

void dram_test::sink() {
  int num_read_commands = NUM_READ_CMDS;
  int i = 0;
  sc_bv<AXIS_USERW> tuser;

  while (i < num_read_commands) {
    if (rx_interface.tready.read() && rx_interface.tvalid.read()) {
      tuser = rx_interface.tuser.read();
      std::cout << "[" << GetSimulationCycle(1.0) << "] Received: addr=" << tuser.range(63, 0).to_uint() << ", data=" << rx_interface.tdata.read().to_uint() << std::endl;
      i++;
    }
    wait();
  }
  sc_stop();
}

void dram_test::assign() {
  rx_interface.tready.write(true);
}

dram_system::dram_system(const sc_module_name& name) : sc_module(name) {
  clk_sig = new sc_clock("clk0", 1.0, SC_NS);

  test_inst = new dram_test("dram_test");
  test_inst->clk(*clk_sig);
  test_inst->rst(rst_sig);

  dut_inst = new dram_axis("dram_axis", 
    "/media/andrew/DATA/research/rad-flow/rad-sim/sim/dram/DRAMsim3/configs/HBM2_4Gb_x128.ini",
    "/media/andrew/DATA/research/rad-flow/rad-sim/build/");
  dut_inst->clk(*clk_sig);
  dut_inst->rst(rst_sig);

  test_to_dut.Connect(test_inst->tx_interface, dut_inst->rx_interface);
  dut_to_test.Connect(dut_inst->tx_interface, test_inst->rx_interface);
}

dram_system::~dram_system() {
  delete clk_sig;
  delete dut_inst;
  delete test_inst;
}