#include <add_driver.hpp>

#define NUM_ADDENDS 100

add_driver::add_driver(const sc_module_name &name)
    : sc_module(name) {

  // Random Seed
  srand (time(NULL));

  // Generate random numbers to be added together by the adder
  std::cout << "Generating Random Numbers to be added ..." << std::endl;
  for (unsigned int i = 0; i < NUM_ADDENDS; i++) {
    unsigned int r_num = std::rand() % 10 + 1;
    std::cout << r_num << " ";
    numbers_to_send.push_back(r_num);
  }
  std::cout << std::endl << "----------------------------------------" << std::endl;

  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

add_driver::~add_driver() {}

void add_driver::source() {
  // Reset
  rst.write(true);
  client_valid.write(false);
  wait();
  rst.write(false);
  wait();

  unsigned int idx = 0;
  while (idx < numbers_to_send.size()) {
    client_tdata.write(numbers_to_send[idx]);
    client_tlast.write(idx >= (numbers_to_send.size() - 1));
    client_valid.write(true);

    wait();

    if (client_valid.read() && client_ready.read()) {
      idx++;
    }
  }
  client_valid.write(false);
  std::cout << "Finished sending all numbers to adder module!" << std::endl;
  wait();
}

void add_driver::sink() {
}