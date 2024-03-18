#pragma once

#include <aximm_interface.hpp>
#include <design_context.hpp>
#include <queue>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <string>
#include <systemc.h>
#include <vector>

#define DATAW 512

class traffic_generator : public RADSimModule {
private:
  std::queue<uint64_t> req_addr_fifo;      // FIFO to store request addresses
  std::queue<sc_bv<DATAW>> req_wdata_fifo; // FIFO to store request data
  std::queue<bool> req_type_fifo; // FIFO to store request type (0: R, 1: W)
  unsigned int req_fifo_depth;    // Depth of request FIFOs
  sc_signal<bool> req_fifo_full;  // Signal flagging request FIFO is full
  sc_signal<bool> aw_accepted; // Signal flagging that AW transaction accepted
  sc_signal<unsigned int> id_counter;  // Counter for transaction IDs
  unsigned int num_received_responses; // Coutnter for received responses
  std::string inst_n;

public:
  sc_in<bool> rst;
  // Interface to driver logic
  sc_in<uint64_t> req_addr;
  sc_in<bool> req_type;
  sc_in<sc_bv<DATAW>> req_wdata;
  sc_in<bool> req_valid;
  sc_out<bool> req_ready;
  sc_out<unsigned int> received_responses;
  // Interface to the NoC
  aximm_master_port aximm_req_interface;

  traffic_generator(const sc_module_name &name, unsigned int fifo_depth, std::string inst_num);
  ~traffic_generator();

  void Assign(); // Combinaational logic process
  void Tick();   // Sequential logic process
  SC_HAS_PROCESS(traffic_generator);
  void RegisterModuleInfo();
};