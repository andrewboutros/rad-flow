#pragma once

#include <systemc.h>

#include <instruction_fifo.hpp>
#include <instructions.hpp>
#include <params.hpp>
#include <sim_utils.hpp>
#include <sstream>
#include <vector>

// Define decoding FSM states
#define DEC_MVU_READ_INST 0
#define DEC_MVU_INIT 1
#define DEC_MVU_CORE 2
#define DEC_EVRF_READ_INST 0
#define DEC_EVRF_INIT 1
#define DEC_EVRF_CORE 2
#define DEC_MFU_READ_INST 0
#define DEC_MFU_INIT 1
#define DEC_MFU_CORE 2
#define DEC_LD_READ_INST 0
#define DEC_LD_INIT 1
#define DEC_LD_CORE 2
#define DEC_LD_TAG_UPDATE 3

// This class defines the top-level instruction dispatcher. This module contains a memory that stores the NPU program
// VLIW instructions and the dispatch logic which directs each portion of the VLIW instruction (macro-op) to its
// corresponding instruction output FIFO to be consumed by the low-level instruction decoders of each NPU block. Each
// block has a separate program counter, so macro-ops can be dispatched to different decoders out of sync
class vliw_inst_dispatch : public sc_module {
 private:
  std::vector<vliw_inst> inst_memory;  // Memory storing VLIW instructions
  // Program counters for NPU blocks
  sc_vector<sc_signal<unsigned int>> sector_pc, evrf_pc, mfu0_pc, mfu1_pc;
  sc_signal<unsigned int> ld_pc;
  // Ready-to-dispatch flags for NPU blocks
  sc_vector<sc_signal<bool>> sector_dispatch_flag, evrf_dispatch_flag, mfu0_dispatch_flag,
      mfu1_dispatch_flag;
  sc_signal<bool> ld_dispatch_flag;

  // Instruction FIFOs, their control and data signals for each of the different NPU blocks
  std::vector<instruction_fifo<mvu_mop>*> sector_mop_fifo;
  sc_vector<sc_signal<bool>> sector_mop_fifo_wen, sector_mop_fifo_full, sector_mop_fifo_almost_full,
      sector_mop_fifo_empty, sector_mop_fifo_almost_empty;
  sc_vector<sc_signal<mvu_mop>> sector_mop_fifo_wdata;

  std::vector<instruction_fifo<evrf_mop>*> evrf_mop_fifo;
  sc_vector<sc_signal<bool>> evrf_mop_fifo_wen, evrf_mop_fifo_full, evrf_mop_fifo_almost_full,
      evrf_mop_fifo_empty, evrf_mop_fifo_almost_empty;
  sc_vector<sc_signal<evrf_mop>> evrf_mop_fifo_wdata;

  std::vector<instruction_fifo<mfu_mop>*> mfu0_mop_fifo;
  sc_vector<sc_signal<bool>> mfu0_mop_fifo_wen, mfu0_mop_fifo_full, mfu0_mop_fifo_almost_full,
      mfu0_mop_fifo_empty, mfu0_mop_fifo_almost_empty;
  sc_vector<sc_signal<mfu_mop>> mfu0_mop_fifo_wdata;

  std::vector<instruction_fifo<mfu_mop>*> mfu1_mop_fifo;
  sc_vector<sc_signal<bool>> mfu1_mop_fifo_wen, mfu1_mop_fifo_full, mfu1_mop_fifo_almost_full,
      mfu1_mop_fifo_empty, mfu1_mop_fifo_almost_empty;
  sc_vector<sc_signal<mfu_mop>> mfu1_mop_fifo_wdata;

  instruction_fifo<ld_mop>* ld_mop_fifo;
  sc_signal<bool> ld_mop_fifo_wen, ld_mop_fifo_full, ld_mop_fifo_almost_full, ld_mop_fifo_empty,
      ld_mop_fifo_almost_empty;
  sc_signal<ld_mop> ld_mop_fifo_wdata;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  // Start and end program counters for the NPU program to be executed
  sc_in<unsigned int> start_pc;
  sc_in<unsigned int> end_pc;
  // Input signal triggering the NPU to start dispatching instructions
  sc_in<bool> start;
  // Interface for writing VLIW instructions to the dipatcher instruction memory
  sc_in<vliw_inst> inst_wdata;
  sc_in<unsigned int> inst_waddr;
  sc_in<bool> inst_wen;
  // Instruction FIFO interfaces for all the NPU blocks
  sc_vector<sc_vector<sc_out<bool>>> sector_mop_rdy;
  sc_vector<sc_vector<sc_in<bool>>> sector_mop_ren;
  sc_vector<sc_vector<sc_out<mvu_mop>>> sector_mop_rdata;
  sc_vector<sc_vector<sc_out<bool>>> evrf_mop_rdy;
  sc_vector<sc_vector<sc_in<bool>>> evrf_mop_ren;
  sc_vector<sc_vector<sc_out<evrf_mop>>> evrf_mop_rdata;
  sc_vector<sc_vector<sc_out<bool>>> mfu0_mop_rdy;
  sc_vector<sc_vector<sc_in<bool>>> mfu0_mop_ren;
  sc_vector<sc_vector<sc_out<mfu_mop>>> mfu0_mop_rdata;
  sc_vector<sc_vector<sc_out<bool>>> mfu1_mop_rdy;
  sc_vector<sc_vector<sc_in<bool>>> mfu1_mop_ren;
  sc_vector<sc_vector<sc_out<mfu_mop>>> mfu1_mop_rdata;
  sc_vector<sc_out<bool>> ld_mop_rdy;
  sc_vector<sc_in<bool>> ld_mop_ren;
  sc_vector<sc_out<ld_mop>> ld_mop_rdata;

  vliw_inst_dispatch(const sc_module_name& name);
  ~vliw_inst_dispatch();
  // Function to dispatche instructions to a given instruction FIFO interface
  void dispatch_logic(sc_signal<bool>& dispatch_flag, sc_signal<unsigned int>& pc,
                      sc_signal<bool>& mop_fifo_almost_full, sc_signal<bool>& mop_fifo_wen, bool op);

  void Tick();
  void Assign();
  SC_HAS_PROCESS(vliw_inst_dispatch);
};

// This class defines the low-level instruction decoder for MVU tiles and the accumulator block which translates an
// incoming macro-op into a sequence of micro-ops to be executed by the corresponding blocks
class sector_and_accum_decoder : public sc_module {
 private:
  instruction_fifo<mvu_mop>* mop_fifo;
  sc_signal<bool> mop_fifo_wen, mop_fifo_ren, mop_fifo_full, mop_fifo_almost_full, mop_fifo_empty,
      mop_fifo_almost_empty;
  sc_signal<mvu_mop> mop_fifo_rdata;

  sc_signal<uint8_t> decode_state;  // FSM state of the decoder logic
  // Counters and signals for implementing the decoding logic
  sc_signal<sc_uint<BATCH_COUNTW>> batch_counter;
  sc_signal<sc_uint<VRF_ADDRW>> vrf_counter, pipeline_counter, vrf_id_counter;
  sc_signal<sc_uint<VRFIDW>> vrf_id;
  sc_signal<sc_uint<MRF_ADDRW>> mrf_chunk_offset, mrf_pipeline_offset, mrf_addr;
  sc_signal<sc_uint<TAGW>> mvu_tag;
  sc_signal<bool> reg_sel;
  sc_signal<sc_uint<NSIZEW>> remaining_words;
  sc_signal<sc_uint<ACCUMIDW>> acc_size;
  sc_signal<mvu_mop> nxt_mvu_inst;  // Macro-op being currently decoded (input to decoding logic)
  mvu_uop decoded_mvu_uop;          // Decoded micro-op (output of decoding logic)
  bool first_uop;

  unsigned int count;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_in<bool> mop_rdy;
  sc_out<bool> mop_ren;
  sc_in<mvu_mop> mop_rdata;
  sc_in<bool> uop_rdy;
  sc_out<bool> uop_wen;
  sc_out<mvu_uop> uop_wdata;

  sector_and_accum_decoder(const sc_module_name& name);
  ~sector_and_accum_decoder();

  void Tick();
  void Assign();
  SC_HAS_PROCESS(sector_and_accum_decoder);
};

// This class defines the low-level instruction decoder for the external VRF block which translates an
// incoming macro-op into a sequence of micro-ops to be executed by the corresponding blocks
class evrf_decoder : public sc_module {
 private:
  instruction_fifo<evrf_mop>* mop_fifo;
  sc_signal<bool> mop_fifo_wen, mop_fifo_ren, mop_fifo_full, mop_fifo_almost_full, mop_fifo_empty,
      mop_fifo_almost_empty;
  sc_signal<evrf_mop> mop_fifo_rdata;

  sc_signal<uint8_t> decode_state;  // FSM state of the decoder logic
  // Counters and signals for implementing the decoding logic
  sc_signal<sc_uint<BATCH_COUNTW>> evrf_batch_counter;
  sc_signal<sc_uint<VRF_ADDRW>> evrf_addr_counter;
  sc_signal<evrf_mop> nxt_evrf_inst;  // Macro-op being currently decoded (input to decoding logic)
  evrf_uop decoded_evrf_uop;          // Decoded micro-op (output of decoding logic)
  bool first_uop;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_in<bool> mop_rdy;
  sc_out<bool> mop_ren;
  sc_in<evrf_mop> mop_rdata;
  sc_in<bool> uop_rdy;
  sc_out<bool> uop_wen;
  sc_out<evrf_uop> uop_wdata;

  evrf_decoder(const sc_module_name& name);
  ~evrf_decoder();

  void Tick();
  void Assign();
  SC_HAS_PROCESS(evrf_decoder);
};

// This class defines the low-level instruction decoder for the multi-function unit block which translates an
// incoming macro-op into a sequence of micro-ops to be executed by the corresponding blocks
class mfu_decoder : public sc_module {
 private:
  instruction_fifo<mfu_mop>* mop_fifo;
  sc_signal<bool> mop_fifo_wen, mop_fifo_ren, mop_fifo_full, mop_fifo_almost_full, mop_fifo_empty,
      mop_fifo_almost_empty;
  sc_signal<mfu_mop> mop_fifo_rdata;

  sc_signal<uint8_t> decode_state;  // FSM state of the decoder logic
  // Counters and signals for implementing the decoding logic
  sc_signal<sc_uint<NSIZEW>> mfu_size;
  sc_signal<sc_uint<TAGW>> mfu_tag;
  sc_signal<sc_uint<7>> mfu_op;
  sc_signal<sc_uint<BATCH_COUNTW>> mfu_batch, mfu_batch_counter;
  sc_signal<sc_uint<VRF_ADDRW>> mfu_counter;
  sc_signal<mfu_mop> nxt_mfu_inst;  // Macro-op being currently decoded (input to decoding logic)
  mfu_uop decoded_mfu_uop;          // Decoded micro-op (output of decoding logic)
  bool first_uop;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_in<bool> mop_rdy;
  sc_out<bool> mop_ren;
  sc_in<mfu_mop> mop_rdata;
  sc_in<bool> uop_rdy;
  sc_out<bool> uop_wen;
  sc_out<mfu_uop> uop_wdata;

  mfu_decoder(const sc_module_name& name);
  ~mfu_decoder();

  void Tick();
  void Assign();
  SC_HAS_PROCESS(mfu_decoder);
};

// This class defines the low-level instruction decoder for the loader block which translates an incoming macro-op into
// a sequence of micro-ops to be executed by the corresponding blocks
class ld_decoder : public sc_module {
 private:
  instruction_fifo<ld_mop>* mop_fifo;
  sc_signal<bool> mop_fifo_wen, mop_fifo_ren, mop_fifo_full, mop_fifo_almost_full, mop_fifo_empty,
      mop_fifo_almost_empty;
  sc_signal<ld_mop> mop_fifo_rdata;

  sc_signal<uint8_t> decode_state;  // FSM state of the decoder logic
  // Counters and signals for implementing the decoding logic
  sc_signal<sc_uint<BLOCK_COUNTW>> ld_tag_counter;
  sc_signal<sc_uint<BATCH_COUNTW>> ld_batch, ld_batch_counter;
  sc_signal<sc_uint<NSIZEW>> ld_size;
  sc_signal<bool> ld_interrupt, ld_report_to_host;
  sc_signal<sc_uint<2>> ld_src_sel;
  sc_signal<sc_uint<VRF_ADDRW>> ld_counter;
  sc_signal<ld_mop> nxt_ld_inst;  // Macro-op being currently decoded (input to decoding logic)
  ld_uop decoded_ld_uop;          // Decoded micro-op (output of decoding logic)
  bool first_uop;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_in<bool> mop_rdy;
  sc_out<bool> mop_ren;
  sc_in<ld_mop> mop_rdata;
  sc_in<bool> uop_rdy;
  sc_out<bool> uop_wen;
  sc_out<ld_uop> uop_wdata;

  ld_decoder(const sc_module_name& name);
  ~ld_decoder();

  void Tick();
  void Assign();
  SC_HAS_PROCESS(ld_decoder);
};