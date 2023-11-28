#include <rtl_add_driver.hpp>

#define NUM_ADDENDS 3

rtl_add_driver::rtl_add_driver(const sc_module_name &name)
    : sc_module(name) {

  // Random Seed
  srand (time(NULL));
  actual_sum = 0;

  // Generate random numbers to be added together by the adder
  std::cout << "Generating Random Numbers to be added ..." << std::endl;
  for (unsigned int i = 0; i < NUM_ADDENDS; i++) {
    unsigned int r_num = std::rand() % 10 + 1;
    std::cout << r_num << " ";
    numbers_to_send.push(r_num);
    actual_sum += r_num;
  }
  std::cout << std::endl << "----------------------------------------" << std::endl;

  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

rtl_add_driver::~rtl_add_driver() {}

void rtl_add_driver::source() {
  // Reset
  rst.write(true);
  client_valid.write(false);
  wait();
  rst.write(false);
  start_cycle = GetSimulationCycle(1.0);
  start_time = std::chrono::steady_clock::now();
  wait();

  while (!numbers_to_send.empty()) {
    client_tdata.write(numbers_to_send.front());
    client_tlast.write(numbers_to_send.size() <= 1);
    client_valid.write(true);

    wait();

    if (client_valid.read() && client_ready.read()) {
      numbers_to_send.pop();
    }
  }
  client_valid.write(false);
  //std::cout << "Finished sending all numbers to client module!" << std::endl;
  wait();
}

void rtl_add_driver::sink() {
  while (!response_valid.read()) {
    wait();
  }
  //std::cout << "Received " << response.read().to_uint64() << " sum from the adder!" << std::endl;
  //std::cout << "The actual sum is " << actual_sum << std::endl;

  if (response.read() != actual_sum) {
    std::cout << "FAILURE - Output is not matching!" << std::endl;
    radsim_design.ReportDesignFailure();
  } else {
    std::cout << "SUCCESS - Output is matching!" << std::endl;
  }

  end_cycle = GetSimulationCycle(1.0);
  end_time = std::chrono::steady_clock::now();
  std::cout << "Simulation Cycles = " << end_cycle - start_cycle << std::endl;
  std::cout << "Simulation Time = " << std::chrono::duration_cast<std::chrono::microseconds> (end_time - start_time).count() << " us" << std::endl;
  NoCTransactionTelemetry::DumpStatsToFile("stats.csv");

  sc_stop();
}