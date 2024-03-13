#include <mult_driver.hpp>

#define NUM_ADDENDS 5

mult_driver::mult_driver(const sc_module_name &name, RADSimDesignContext* radsim_design)
    : sc_module(name) {
  
  this->radsim_design_ = radsim_design; //AKB ADDED: update member for later use

  //for simulation cycle count
  start_cycle = 0;
  end_cycle = 0;

  // Random Seed
  srand (time(NULL));
  actual_product = 1;

  // Generate random numbers to be multiplied together by the multiplier
  std::cout << "Generating Random Numbers to be multiplied ..." << std::endl;
  for (unsigned int i = 0; i < NUM_ADDENDS; i++) {
    unsigned int r_num = std::rand() % 10 + 1;
    std::cout << r_num << " ";
    numbers_to_send.push(r_num);
    actual_product *= r_num;
  }
  std::cout << std::endl << "----------------------------------------" << std::endl;

  SC_CTHREAD(source, clk.pos());
  SC_CTHREAD(sink, clk.pos());
}

mult_driver::~mult_driver() {}

void mult_driver::source() {
  // Reset
  rst.write(true);
  client_valid.write(false);
  wait();
  rst.write(false);
  start_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
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
  std::cout << "Finished sending all numbers to client module!" << std::endl;
  wait();
}

void mult_driver::sink() {
  //works, temp commented out to test, returned now
  while (!(response_valid.read())) { //&& mult_inter_rad_recvd.read())) {
    wait();
  }
  std::cout << "Received " << response.read().to_uint64() << " product from the multiplier!" << std::endl;
  std::cout << "The actual product is " << actual_product << std::endl;

  if (response.read() != actual_product) std::cout << "FAILURE - Output is not matching!" << std::endl;
  else std::cout << "SUCCESS - Output is matching!" << std::endl;

  end_cycle = GetSimulationCycle(radsim_config.GetDoubleKnob("sim_driver_period"));
  std::cout << "Simulation Cycles for Just Mult Portion = " << end_cycle - start_cycle << std::endl;


  //sc_stop(); //AKB: replaced with setting flag
  this->radsim_design_->set_rad_done(); //AKB ADDED: flag to replace sc_stop calls
  return; //AKB ADDED

}