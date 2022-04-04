#pragma once

#include <math.h>
#include <systemc.h>

#include <instructions.hpp>
#include <params.hpp>
#include <queue>
#include <sim_utils.hpp>

// Enum for specifying the type of communication channel for each of the AXI-streaming adapters -- depending on the
// type, the adapters convert the FIFO interface to an AXI-streaming interface in a different way. For example, write-
// back channels have vector FIFO interfaces including data, destination, address and end flag. On the other hand,
// instruction channels have different encoding/decoding into/from a bitvector compare to that of data channels.
enum npu_channel_type { feedforward, writeback_narrow, writeback_wide, inst };

// Adapter class for wrapping a number of FIFO output interfaces working in lockstep (rdata, rdy,
// ren) as an AXI-stream master inteface. The FIFO output data type and its equivalent bitvector
// representation are templated to be compatible with different NPU channel types.
template <typename fifo_type, typename bv_type>
class axis_master_fifo_adapter : public sc_module {
 private:
  npu_channel_type interface_type;  // Type of interface {data, instruction, writeback}
  unsigned int interface_dataw;     // Bitwidth of the AXI-S data field
  unsigned int num_fifo;            // Number of FIFOs in this interface (useful in case of multiple cores in lockstep)
  unsigned int payload_bitwidth;    // Bitwidth of payload to be communicated
  unsigned int transfers_per_axis_packet;            // Number of transfers needed to communicate a complete payload
  unsigned int element_bitwidth;                     // Bitwidth of a single element of the vector interface input
  unsigned int destination;                          // Destination ID for communication
  unsigned int stream_id;                            // Stream ID for communication
  std::queue<sc_bv<AXIS_TRANSACTION_WIDTH>> buffer;  // Adapter buffer for width adaptation
  sc_bv<AXIS_TRANSACTION_WIDTH> buffer_wdata;        // Buffer write data
  bv_type payload_bitvector;                         // Bitvector of the payload to be communicated
  unsigned int buffer_capacity;                      // Capacity of adapter buffer
  sc_signal<unsigned int> buffer_occupancy;          // Current occupancy of adapter buffer

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_vector<sc_in<bool>> fifo_rdy;
  sc_vector<sc_out<bool>> fifo_ren;
  sc_vector<sc_in<fifo_type>> fifo_rdata;
  axis_master_port axis_port;

  axis_master_fifo_adapter(const sc_module_name& _name, npu_channel_type _interface_type, unsigned int _interface_dataw,
                           unsigned int _num_fifo, unsigned int _element_bitwidth, unsigned int _destination,
                           unsigned int _stream_id);
  ~axis_master_fifo_adapter();

  bool buffer_full();                 // Checks if adapter buffer is full
  void data_fifo_to_sc_bitvector();   // Converts a data FIFO entry to a SystemC bitvector
  void inst_fifo_to_sc_bitvector();   // Converts an instruction FIFO entry to a SystemC bitvector
  void insert_payload_into_buffer();  // Pushes a payload into the adapter buffer
  void Tick();
  void Assign();

  SC_HAS_PROCESS(axis_master_fifo_adapter);
};

// Adapter class for wrapping a number of FIFO input interfaces working in lockstep (rdata, rdy,
// ren) as an AXI-stream slave inteface. The FIFO output data type and its equivalent bitvector
// representation are templated to be compatible with different NPU channel types.
template <typename fifo_type, typename bv_type>
class axis_slave_fifo_adapter : public sc_module {
 private:
  npu_channel_type interface_type;  // Type of interface {data, instruction, writeback}
  unsigned int interface_dataw;
  unsigned int num_fifo;          // Number of FIFOs in this interface (useful in case of multiple cores in lockstep)
  unsigned int payload_bitwidth;  // Bitwidth of payload to be received
  unsigned int transfers_per_axis_packet;    // Number of transfers needed to receive a complete payload
  unsigned int element_bitwidth;             // Bitwidth of a single element of the vector interface output
  unsigned int num_element;                  // Number of elements of the vector interface output
  std::queue<bv_type> buffer;                // Adapter buffer for width adaptation
  bv_type buffer_wdata;                      // Buffer write data
  bv_type payload_bitvector;                 // Bitvector of the payload to be received
  unsigned int buffer_capacity;              // Capacity of adapter buffer
  sc_signal<unsigned int> buffer_occupancy;  // Current occupancy of adapter buffer
  sc_signal<unsigned int> transfer_count;    // Count of received-so-far transfers (for a specific payload)

  unsigned int count = 0;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_vector<sc_out<bool>> fifo_rdy;
  sc_vector<sc_in<bool>> fifo_ren;
  sc_vector<sc_out<fifo_type>> fifo_rdata;
  axis_slave_port axis_port;

  axis_slave_fifo_adapter(const sc_module_name& _name, npu_channel_type _interface_type, unsigned int _interface_dataw,
                          unsigned int _num_fifo, unsigned int _element_bitwidth, unsigned int _num_element);
  ~axis_slave_fifo_adapter();

  void sc_bitvector_to_data_fifo();  // Converts a SystemC bitvector to a data FIFO entry
  void sc_bitvector_to_inst_fifo();  // Converts a SystemC bitvector to an instruction FIFO entry
  void Tick();
  void Assign();
  SC_HAS_PROCESS(axis_slave_fifo_adapter);
};