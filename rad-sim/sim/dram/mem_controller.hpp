#pragma once

#include <systemc.h>

#include <aximm_interface.hpp>
#include <bitset>
#include <cmath>
#include <design_context.hpp>
#include <functional>
#include <memory_system.h>
#include <queue>
#include <radsim_utils.hpp>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <vector>

class mem_controller : public radsim_module {
private:
  // The DRAMsim memory system model
  dramsim3::MemorySystem *_dramsim;
  unsigned int _mem_id;
  // Number of memory channels and interfaces of the controller (specified by
  // DRAMsim)
  unsigned int _num_channels;
  // Bit width of each memory channel and AXI-MM interface
  unsigned int _memory_channel_bitwidth;
  unsigned int _controller_channel_bitwidth;
  // Clock period of the memory (specified by DRAMsim) and the controller
  // The ratio between these clock periods has to match that of the
  // interface/channel bitwidths for rate matching
  double _memory_clk_period_ns;
  double _controller_clk_period_ns;
  // Setting priority to read or write interfaces
  bool _read_before_write;
  // Book-keeping of memory contents for each channel (hashtable of address ->
  // data)
  std::vector<std::unordered_map<uint64_t, sc_bv<AXI4_MAX_DATAW>>>
      _mem_contents;
  // Input request queues expanding burst transactions into multiple read/write
  // For each channel, keep a queue of address & is_write pair
  std::vector<std::queue<std::pair<uint64_t, bool>>> _write_address_queue;
  std::vector<std::queue<std::pair<uint64_t, sc_bv<AXI4_MAX_DATAW>>>>
      _write_data_queue;
  std::vector<std::queue<std::tuple<uint64_t, uint64_t, bool>>>
      _read_address_queue;
  // Queue for requests already dispatched to DRAMsim but waiting for responses
  // (assuming in-order) For each channel, keep a queue of response address (and
  // data if write request)
  std::vector<std::queue<std::tuple<uint64_t, sc_bv<AXI4_MAX_DATAW>, bool>>>
      _outstanding_write_requests;
  std::vector<std::queue<std::tuple<uint64_t, uint64_t, bool>>>
      _outstanding_read_requests;
  std::vector<std::unordered_map<uint64_t, unsigned int>>
      _out_of_order_read_requests;
  // Queue for responses populated by read/write callback functions
  std::vector<std::queue<std::tuple<uint64_t, sc_bv<AXI4_MAX_DATAW>, bool>>>
      _output_read_responses;
  std::vector<std::queue<uint64_t>> _output_write_responses;
  // Sizes and current occupancy of different queues
  unsigned int _input_queue_size, _output_queue_size;
  sc_vector<sc_signal<unsigned int>> _write_address_queue_occupancy,
      _write_data_queue_occupancy, _read_address_queue_occupancy;
  sc_vector<sc_signal<unsigned int>> _num_outstanding_write_requests,
      _num_outstanding_read_requests;
  sc_vector<sc_signal<unsigned int>> _output_write_queue_occupancy,
      _output_read_queue_occupancy;
  // Some memory parameters
  unsigned int _total_mem_capacity_megabytes;
  uint64_t _total_num_addressable_words, _total_num_addressable_words_M;
  unsigned int _num_ranks, _num_bank_groups, _num_banks_per_group, _num_rows,
      _num_word_cols;
  unsigned int _addressable_size_bytes;
  // Address mapping
  std::unordered_map<std::string, unsigned int> _field_widths;
  std::vector<std::string> _mem_addr_fields;

  std::vector<unsigned int> _debug_sent_responses_counter;
  std::vector<unsigned int> _debug_received_requests_counter;

public:
  sc_in<bool> mem_clk;
  sc_in<bool> rst;
  sc_vector<aximm_slave_port> mem_channels;

  mem_controller(const sc_module_name &name, unsigned int dram_id,
                 std::string init_filename = "");
  ~mem_controller();

  void MemReadCallback(uint64_t addr);
  void MemWriteCallback(uint64_t addr);
  void PrintMemParameters();
  void PrintStats();
  uint64_t AddressMapping(uint64_t addr, unsigned int channel_id);
  unsigned int ChannelFromAddr(uint64_t addr);
  unsigned int GetNumChannels();
  unsigned int GetMemCapacity();
  unsigned int GetAddressableWordSize();
  void InitializeMemoryContents(std::string &init_filename);

  void Assign();
  void Tick();
  void MemTick();
  SC_HAS_PROCESS(mem_controller);

  void RegisterModuleInfo();
};
