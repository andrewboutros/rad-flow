#include <add_driver.hpp>

#define NUM_ADDENDS 3

add_driver::add_driver(const sc_module_name &name)
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
    numbers_to_send.push(r_num); //ADDED, push twice (one for each adder inst)
    actual_sum += r_num;
  }
  std::cout << std::endl << "----------------------------------------" << std::endl;

  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

add_driver::~add_driver() {}

bool local_sel = 0; //ADDED
void add_driver::source() {
  // Reset
  rst.write(true);
  client_valid.write(false);
  wait();
  rst.write(false);
  wait();

  while (!numbers_to_send.empty()) {
    client_tdata.write(numbers_to_send.front());
    client_tlast.write(numbers_to_send.size() <= 1);
    client_valid.write(true);


    //ADDED
    client_tsel_data.write(local_sel);
    client_tsel_valid.write(true);
    if (local_sel == 0) { //flip to adder 2 for next number
      local_sel = 1;
    }
    else { //flip to adder 1 for next number
      local_sel = 0;
    }


    wait();

    if (client_valid.read() && client_ready.read()) {
      numbers_to_send.pop();
    }
  }
  client_valid.write(false);
  std::cout << "Finished sending all numbers to client module!" << std::endl;
  wait();
}

void add_driver::sink() {
  while (!response_valid.read()) {
    wait();
  }
  std::cout << "Received " << response.read().to_uint64() << " sum from the adder!" << std::endl;
  std::cout << "The actual sum is " << actual_sum << std::endl;

  if (response.read() != actual_sum) std::cout << "FAILURE - Output is not matching!" << std::endl;
  else std::cout << "SUCCESS - Output is matching!" << std::endl;

  //ADDED:
  while (!response_valid2.read()) {
    wait();
  }
  std::cout << "Received " << response2.read().to_uint64() << " sum from the adder2!" << std::endl;
  std::cout << "The actual sum is " << actual_sum << std::endl;

  if (response2.read() != actual_sum) std::cout << "FAILURE - Output2 is not matching!" << std::endl;
  else std::cout << "SUCCESS - Output2 is matching!" << std::endl;

  sc_stop();
}