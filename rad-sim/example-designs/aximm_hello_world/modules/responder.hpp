#pragma once

#include <aximm_interface.hpp>
#include <design_context.hpp>
#include <queue>
#include <radsim_defines.hpp>
#include <radsim_module.hpp>
#include <requester.hpp>
#include <string>
#include <systemc.h>
#include <vector>

class responder : public RADSimModule {
private:
  std::queue<uint64_t> req_waddr_fifo; // FIFO to store write request addresses
  std::queue<sc_bv<DATAW>> req_wdata_fifo; // FIFO to store write request data
  std::queue<uint64_t> req_baddr_fifo;  // FIFO to store B address of W request
  std::queue<uint64_t> req_araddr_fifo; // FIFO to store read request addresses
  std::queue<uint64_t> req_raddr_fifo;  // FIFO to store R address of AR request
  unsigned int req_fifo_depth;          // Depth of request FIFOs
  sc_signal<bool> write_req_fifo_full;  // Write request FIFO is full
  sc_signal<bool> write_data_fifo_full; // Write data FIFO is full
  sc_signal<bool> read_req_fifo_full;   // Read request FIFO is full

  std::queue<uint64_t> resp_addr_fifo;     // FIFO to hold response addresses
  std::queue<sc_bv<DATAW>> resp_data_fifo; // FIFO to hold response data
  std::queue<bool> resp_type_fifo;    // FIFO to hold response type (0:R, 1:B)
  unsigned int resp_fifo_depth;       // Depth of response FIFOs
  sc_signal<bool> resp_fifo_empty;    // Response FIFO is empty
  sc_signal<bool> resp_fifo_full;     // Response FIFO is full
  sc_signal<unsigned int> id_counter; // Counter for transaction ID

  // Storage for pairs of address & data
  std::unordered_map<uint64_t, sc_bv<DATAW>> storage;

public:
  sc_in<bool> rst;
  // Interface to the NoC
  aximm_slave_port aximm_resp_interface;

  responder(const sc_module_name &name, unsigned int fifo_depth);
  ~responder();

  void Assign(); // Combinational logic process
  void Tick();   // Sequential logic process
  SC_HAS_PROCESS(responder);
  void RegisterModuleInfo();
};