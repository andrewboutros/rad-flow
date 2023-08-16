#include "axis_fifo_adapters.hpp"

// AXI-stream master adapter constructor
template <typename fifo_type, typename bv_type>
axis_master_fifo_adapter<fifo_type, bv_type>::axis_master_fifo_adapter(
    const sc_module_name& _name, unsigned int _interface_type, unsigned int _interface_dataw, unsigned int _num_fifo,
    unsigned int _element_bitwidth, std::string& _destination_port)
    : sc_module(_name), fifo_rdy("fifo_rdy"), fifo_ren("fifo_ren"), fifo_rdata("fifo_rdata") {
  // Initialize member variables
  interface_type = _interface_type;
  if (_interface_dataw > AXIS_MAX_DATAW)
    sim_log.log(error, "AXI-S datawidth exceeds maximum value!", this->name());
  interface_dataw = _interface_dataw;
  num_fifo = _num_fifo;
  payload_bitvector = 0;
  payload_bitwidth = payload_bitvector.length();
  if (interface_type == VEW_WRITEBACK_INTERFACE) {
    transfers_per_axis_packet = (int)ceil(1.0 * NARROW_WRITEBACK_WIDTH / interface_dataw);
  } else if (interface_type == MVU_WRITEBACK_INTERFACE) {
    transfers_per_axis_packet = (int)ceil(1.0 * WIDE_WRITEBACK_WIDTH / interface_dataw);
  } else {
    transfers_per_axis_packet = (int)ceil(1.0 * payload_bitwidth / interface_dataw);
  }
  element_bitwidth = _element_bitwidth;
  destination_port = _destination_port;
  buffer_capacity = AXIS_ADAPTER_BUFFER_CAPACITY * transfers_per_axis_packet;

  // Initialize the FIFO interface vectors
  init_vector<sc_in<bool>>::init_sc_vector(fifo_rdy, num_fifo);
  init_vector<sc_out<bool>>::init_sc_vector(fifo_ren, num_fifo);
  init_vector<sc_in<fifo_type>>::init_sc_vector(fifo_rdata, num_fifo);

  // Set the sensitivity list of the Assign SC_METHOD & clk and reset of the Tick SC_CTHREAD
  SC_METHOD(Assign);
  sensitive << buffer_occupancy << fifo_rdy[0];
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

// AXI-stream master adapter destructor
template <typename fifo_type, typename bv_type>
axis_master_fifo_adapter<fifo_type, bv_type>::~axis_master_fifo_adapter() {}

// A function for checking if the AXI-stream adapter buffer is full
template <typename fifo_type, typename bv_type>
bool axis_master_fifo_adapter<fifo_type, bv_type>::buffer_full() {
  return (buffer_occupancy.read() > buffer_capacity - transfers_per_axis_packet);
}

// A function for converting the data FIFO interface (data_vector) into a bitvector to be
// transferred over one or multiple AXI-stream transfers in a packet
template <typename fifo_type, typename bv_type>
void axis_master_fifo_adapter<fifo_type, bv_type>::data_fifo_to_sc_bitvector() {
  unsigned int idx_offset, start_idx, end_idx;
  fifo_type fifo_data_vector = fifo_rdata[0].read();
  unsigned int fifo_size = fifo_data_vector.size();
  unsigned int element_id_offset = 0;

  // Adjust start element if this inteface is for an NPU writeback channel since the first three
  // elements of the data vector contain the VRF ID, VRF write address, and tag update flag
  if (interface_type == VEW_WRITEBACK_INTERFACE || interface_type == MVU_WRITEBACK_INTERFACE) {
    fifo_size -= 4;
    element_id_offset = 4;
  }

  // Loop over FIFO interfaces, get data vectors & calculate index offset for this interface
  for (unsigned int fifo_id = 0; fifo_id < num_fifo; fifo_id++) {
    fifo_type fifo_data_vector = fifo_rdata[fifo_id].read();
    idx_offset = fifo_id * fifo_size * element_bitwidth;

    // Loop over elements in the FIFO data vector & set the corresponding range of the bitvector
    for (unsigned int element_id = 0; element_id < fifo_size; element_id++) {
      start_idx = idx_offset + (element_id * element_bitwidth);
      end_idx = idx_offset + ((element_id + 1) * element_bitwidth) - 1;
      payload_bitvector.range(end_idx, start_idx) = fifo_data_vector[element_id_offset + element_id];
    }
  }

  // Add the VRF ID, VRF write address, and tag update flag in the most significant bits of the
  // payload bitvector if this interface is for an NPU writeback channel
  if (interface_type == VEW_WRITEBACK_INTERFACE) {
    fifo_type fifo_data_vector = fifo_rdata[0].read();
    unsigned int vrf_id_start_idx = payload_bitwidth - VRF_WB_SELW;
    unsigned int addr_start_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW;
    unsigned int flag_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW - 1;
    unsigned int block_id_start_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW - BLOCK_WB_SELW - 1;
    // Subtract 1 from the destination VRF ID to change from one-indexed to zero-indexed
    payload_bitvector.range(vrf_id_start_idx + VRF_WB_SELW - 1, vrf_id_start_idx) = fifo_data_vector[0];
    payload_bitvector.range(addr_start_idx + VRF_ADDRW - 1, addr_start_idx) = fifo_data_vector[1];
    payload_bitvector.set_bit(flag_idx, fifo_data_vector[2]);
    payload_bitvector.range(block_id_start_idx + BLOCK_WB_SELW - 1, block_id_start_idx) = fifo_data_vector[3];
  } else if (interface_type == MVU_WRITEBACK_INTERFACE) {
    fifo_type fifo_data_vector = fifo_rdata[0].read();
    unsigned int vrf_id_start_idx = payload_bitwidth - VRF_WB_SELW;
    unsigned int addr_start_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW;
    unsigned int flag_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW - 1;
    // Subtract 1 from the destination VRF ID to change from one-indexed to zero-indexed
    payload_bitvector.range(vrf_id_start_idx + VRF_WB_SELW - 1, vrf_id_start_idx) = fifo_data_vector[0];
    payload_bitvector.range(addr_start_idx + LOW_PRECISION - 1, addr_start_idx) = fifo_data_vector[1];
    if (VRF_ADDRW > LOW_PRECISION)
      payload_bitvector.range(addr_start_idx + VRF_ADDRW - 1, addr_start_idx + LOW_PRECISION) =
          fifo_data_vector[2].range(VRF_ADDRW - 1 - LOW_PRECISION, 0);
    payload_bitvector.set_bit(flag_idx, fifo_data_vector[3]);
  }
}

// A function for converting the instruction FIFO interface (mvu_mop, evrf_mop, etc.) into a
// bitvector to be transferred over one or multiple AXI-stream transfers in a packet
template <typename fifo_type, typename bv_type>
void axis_master_fifo_adapter<fifo_type, bv_type>::inst_fifo_to_sc_bitvector() {
  fifo_type fifo_inst = fifo_rdata[0].read();
  payload_bitvector = fifo_inst.convert_to_bv();
}

// A function for splitting the payload bitvector into transfers over the AXI-stream interface --
// the number of transfers per payload depends on the AXI-stream data width and payload width
template <typename fifo_type, typename bv_type>
void axis_master_fifo_adapter<fifo_type, bv_type>::insert_payload_into_buffer() {
  // Calculate ID, address, and flag start indecies to use in case of a writeback interface
  unsigned int vrf_id_start_idx = payload_bitwidth - VRF_WB_SELW;
  unsigned int addr_start_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW;
  unsigned int flag_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW - 1;
  unsigned int block_id_start_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW - BLOCK_WB_SELW - 1;

  // Break each payload into multiple transfers (depending on width of the AXI-stream interface)
  // and insert them into the buffer. This models an asymmetric FIFO behaviour if AXI-stream data
  // bitwidth is less than the payload bitwidth.
  for (unsigned int transfer_id = 0; transfer_id < transfers_per_axis_packet; transfer_id++) {
    // Calculate the payload start and end indecies for this transfer
    unsigned int start_idx = transfer_id * interface_dataw;
    unsigned int end_idx = std::min((transfer_id + 1) * interface_dataw, payload_bitwidth);

    // Set different fields of the AXI-stream transfer. In case of an NPU writeback interface, the
    // destination ID is part of the payload and write address is specified in the TUSER field.
    AXIS_TDATA(buffer_wdata) = payload_bitvector.range(end_idx - 1, start_idx);
    AXIS_TSTRB(buffer_wdata) = (int)pow(2, AXIS_STRBW) - 1;
    AXIS_TKEEP(buffer_wdata) = (int)pow(2, AXIS_KEEPW) - 1;
    if (interface_type == MVU_WRITEBACK_INTERFACE) {
      TUSER_FLAG(buffer_wdata) = payload_bitvector.get_bit(flag_idx);
      TUSER_ADDR(buffer_wdata) = payload_bitvector.range(addr_start_idx + VRF_ADDRW - 1, addr_start_idx);
      TUSER_VRFID(buffer_wdata) = payload_bitvector.range(vrf_id_start_idx + VRF_WB_SELW - 1, vrf_id_start_idx);
      AXIS_TDEST(buffer_wdata) = radsim_design.GetPortDestinationID(destination_port);
    } else if (interface_type == VEW_WRITEBACK_INTERFACE) {
      TUSER_FLAG(buffer_wdata) = payload_bitvector.get_bit(flag_idx);
      TUSER_ADDR(buffer_wdata) = payload_bitvector.range(addr_start_idx + VRF_ADDRW - 1, addr_start_idx);
      TUSER_VRFID(buffer_wdata) = payload_bitvector.range(vrf_id_start_idx + VRF_WB_SELW - 1, vrf_id_start_idx);
      AXIS_TDEST(buffer_wdata) = payload_bitvector.range(block_id_start_idx + BLOCK_WB_SELW - 1, block_id_start_idx);
    } else {
      TUSER_FLAG(buffer_wdata) = 0;
      TUSER_ADDR(buffer_wdata) = 0;
      AXIS_TDEST(buffer_wdata) = radsim_design.GetPortDestinationID(destination_port);
    }
    AXIS_TID(buffer_wdata) = radsim_design.GetPortInterfaceID(destination_port);
    AXIS_TLAST(buffer_wdata) = (transfer_id == transfers_per_axis_packet - 1);
    buffer.push(buffer_wdata);
    assert(buffer.size() <= buffer_capacity);
    //std::cout << "Destination Port " << destination_port << " is at node " << radsim_design.GetPortDestinationID(destination_port) << " interface " << radsim_design.GetPortInterfaceID(destination_port) << std::endl;
  }
}

// Sequential logic of the AXI-stream master adapter interface
template <typename fifo_type, typename bv_type>
void axis_master_fifo_adapter<fifo_type, bv_type>::Tick() {
  // Reset logic
  axis_port.Reset();
  while (!buffer.empty()) buffer.pop();
  buffer_occupancy.write(0);
  wait();

  // Sequential logic
  while (true) {
    bool pushed_into_buffer = false;
    bool popped_from_buffer = false;

    // If the FIFO is ready and the adapter buffer is not full, put FIFO data in bitvector format
    // and push it into the buffer as one or more AXI-stream transfers based on the AXI data width
    if (fifo_rdy[0].read() && !buffer_full()) {
      if (interface_type == FEEDFORWARD_INTERFACE || interface_type == VEW_WRITEBACK_INTERFACE || interface_type == MVU_WRITEBACK_INTERFACE) {
        data_fifo_to_sc_bitvector();
      } else {
        inst_fifo_to_sc_bitvector();
      }
      insert_payload_into_buffer();

      // Set flag for pushing into buffer and write to adapter ports
      pushed_into_buffer = true;
      axis_port.tvalid.write(true);
      sc_bv<AXIS_TRANSACTION_WIDTH> buffer_front = buffer.front();
      axis_port.tdata.write(AXIS_TDATA(buffer_front));
      axis_port.tstrb.write(AXIS_TSTRB(buffer_front));
      axis_port.tkeep.write(AXIS_TKEEP(buffer_front));
      axis_port.tuser.write(AXIS_TUSER(buffer_front));
      axis_port.tdest.write(AXIS_TDEST(buffer_front));
      axis_port.tid.write(AXIS_TID(buffer_front));
      if (AXIS_TLAST(buffer_front) == 1)
        axis_port.tlast.write(true);
      else
        axis_port.tlast.write(false);
    }

    // When a transfer happens (i.e., tvalid & tready are asserted), pop from the buffer and update
    // the AXI-stream adapter ports
    if (axis_port.tvalid.read() && axis_port.tready.read()) {
      popped_from_buffer = true;
      buffer.pop();
      assert(buffer_occupancy.read() > 0);

      axis_port.tvalid.write(!buffer.empty());
      if (!buffer.empty()) {
        sc_bv<AXIS_TRANSACTION_WIDTH> buffer_front = buffer.front();
        axis_port.tdata.write(AXIS_TDATA(buffer_front));
        axis_port.tstrb.write(AXIS_TSTRB(buffer_front));
        axis_port.tkeep.write(AXIS_TKEEP(buffer_front));
        axis_port.tuser.write(AXIS_TUSER(buffer_front));
        axis_port.tdest.write(AXIS_TDEST(buffer_front));
        axis_port.tid.write(AXIS_TID(buffer_front));
        if (AXIS_TLAST(buffer_front) == 1)
          axis_port.tlast.write(true);
        else
          axis_port.tlast.write(false);
      }
    }

    // Updated buffer occupancy depending on what happened this cycle
    if (pushed_into_buffer && popped_from_buffer) {
      buffer_occupancy.write(buffer_occupancy.read() + transfers_per_axis_packet - 1);
    } else if (pushed_into_buffer) {
      buffer_occupancy.write(buffer_occupancy.read() + transfers_per_axis_packet);
    } else if (popped_from_buffer) {
      buffer_occupancy.write(buffer_occupancy.read() - 1);
    }
    wait();
  }
}

// Combinational logic of the AXI-stream master adapter interface
template <typename fifo_type, typename bv_type>
void axis_master_fifo_adapter<fifo_type, bv_type>::Assign() {
  // Setting FIFO read enable signals
  bool ren = fifo_rdy[0].read() && !buffer_full();
  for (unsigned int fifo_id = 0; fifo_id < num_fifo; fifo_id++) fifo_ren[fifo_id].write(ren);
}

// Declarations of needed AXI-stream master adapter template types
template class axis_master_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>;
template class axis_master_fifo_adapter<data_vector<tb_output_precision>, sc_bv<NARROW_WRITEBACK_BV_WIDTH>>;
template class axis_master_fifo_adapter<data_vector<tb_input_precision>, sc_bv<WIDE_WRITEBACK_BV_WIDTH>>;
template class axis_master_fifo_adapter<data_vector<tb_output_precision>, sc_bv<WIDE_WRITEBACK_BV_WIDTH>>;
template class axis_master_fifo_adapter<mvu_mop, sc_bv<MVU_MOP_BITWIDTH>>;
template class axis_master_fifo_adapter<evrf_mop, sc_bv<EVRF_MOP_BITWIDTH>>;
template class axis_master_fifo_adapter<mfu_mop, sc_bv<MFU_MOP_BITWIDTH>>;
template class axis_master_fifo_adapter<ld_mop, sc_bv<LD_MOP_BITWIDTH>>;

// AXI-stream slave adapter constructor
template <typename fifo_type, typename bv_type>
axis_slave_fifo_adapter<fifo_type, bv_type>::axis_slave_fifo_adapter(const sc_module_name& _name,
                                                                     unsigned int _interface_type,
                                                                     unsigned int _interface_dataw,
                                                                     unsigned int _num_fifo,
                                                                     unsigned int _element_bitwidth,
                                                                     unsigned int _num_element)
    : sc_module(_name), fifo_rdy("fifo_rdy"), fifo_ren("fifo_ren"), fifo_rdata("fifo_rdata") {
  // Initialize member variables
  interface_type = _interface_type;
  if (_interface_dataw > AXIS_MAX_DATAW)
    sim_log.log(error, "AXI-S datawidth exceeds maximum value!", this->name());
  interface_dataw = _interface_dataw;
  num_fifo = _num_fifo;
  payload_bitvector = 0;
  buffer_wdata = 0;
  payload_bitwidth = payload_bitvector.length();
  if (interface_type == VEW_WRITEBACK_INTERFACE) {
    transfers_per_axis_packet = (int)ceil(1.0 * NARROW_WRITEBACK_WIDTH / interface_dataw);
  } else if (interface_type == MVU_WRITEBACK_INTERFACE) {
    transfers_per_axis_packet = (int)ceil(1.0 * WIDE_WRITEBACK_WIDTH / interface_dataw);
  } else {
    transfers_per_axis_packet = (int)ceil(1.0 * payload_bitwidth / interface_dataw);
  }
  element_bitwidth = _element_bitwidth;
  num_element = _num_element;
  buffer_capacity = AXIS_ADAPTER_BUFFER_CAPACITY;

  // Initialize the FIFO interface vectors
  init_vector<sc_out<bool>>::init_sc_vector(fifo_rdy, num_fifo);
  init_vector<sc_in<bool>>::init_sc_vector(fifo_ren, num_fifo);
  init_vector<sc_out<fifo_type>>::init_sc_vector(fifo_rdata, num_fifo);

  // Set the sensitivity list of the Assign SC_METHOD & clk and reset of the Tick SC_CTHREAD
  SC_METHOD(Assign);
  sensitive << buffer_occupancy << rst;
  for (unsigned int fifo_id = 0; fifo_id < num_fifo; fifo_id++) {
    sensitive << fifo_rdata[fifo_id];
  }
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

// AXI-stream slave adapter destructor
template <typename fifo_type, typename bv_type>
axis_slave_fifo_adapter<fifo_type, bv_type>::~axis_slave_fifo_adapter() {}

// A function for converting the incoming payload bitvector into data FIFO interface (data_vector)
// to be consumed by the receiving module
template <typename fifo_type, typename bv_type>
void axis_slave_fifo_adapter<fifo_type, bv_type>::sc_bitvector_to_data_fifo() {
  payload_bitvector = buffer.front();
  unsigned int vrf_id_start_idx, addr_start_idx, flag_idx, block_id_start_idx;
  unsigned int fifo_size = num_element;
  unsigned int element_id_offset = 0;
  if (interface_type == VEW_WRITEBACK_INTERFACE || interface_type == MVU_WRITEBACK_INTERFACE) {
    fifo_size += 4;
    element_id_offset = 4;
    vrf_id_start_idx = payload_bitwidth - VRF_WB_SELW;
    addr_start_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW;
    flag_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW - 1;
    block_id_start_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW - 1 - BLOCK_WB_SELW;
  }

  for (unsigned int fifo_id = 0; fifo_id < num_fifo; fifo_id++) {
    fifo_type fifo_data_vector(fifo_size);
    unsigned int idx_offset = fifo_id * num_element * element_bitwidth;
    for (unsigned int element_id = 0; element_id < num_element; element_id++) {
      unsigned int start_idx = idx_offset + (element_id * element_bitwidth);
      unsigned int end_idx = idx_offset + ((element_id + 1) * element_bitwidth);
      fifo_data_vector[element_id_offset + element_id] = payload_bitvector.range(end_idx - 1, start_idx).to_int();
    }
    if (interface_type == VEW_WRITEBACK_INTERFACE) {
      fifo_data_vector[0] = payload_bitvector.range(vrf_id_start_idx + VRF_WB_SELW - 1, vrf_id_start_idx).to_uint();
      fifo_data_vector[1] = payload_bitvector.range(addr_start_idx + VRF_ADDRW - 1, addr_start_idx).to_uint();
      if (payload_bitvector.get_bit(flag_idx))
        fifo_data_vector[2] = 1;
      else
        fifo_data_vector[2] = 0;
      fifo_data_vector[3] = payload_bitvector.range(block_id_start_idx + BLOCK_WB_SELW - 1, block_id_start_idx).to_uint();
    } else if (interface_type == MVU_WRITEBACK_INTERFACE) {
      fifo_data_vector[0] = payload_bitvector.range(vrf_id_start_idx + VRF_WB_SELW - 1, vrf_id_start_idx).to_uint();
      fifo_data_vector[1] = payload_bitvector.range(addr_start_idx + LOW_PRECISION - 1, addr_start_idx).to_uint();
      if (VRF_ADDRW > LOW_PRECISION)
        fifo_data_vector[2] =
            payload_bitvector.range(addr_start_idx + VRF_ADDRW - 1, addr_start_idx + LOW_PRECISION).to_uint();
      if (payload_bitvector.get_bit(flag_idx))
        fifo_data_vector[3] = 1;
      else
        fifo_data_vector[3] = 0;
    }
    fifo_rdata[fifo_id].write(fifo_data_vector);
  }
}

// A function for converting the incoming payload bitvector into instruction FIFO interface
// (mvu_mop, evrf_mop, etc.) to be consumed by the receiving module
template <typename fifo_type, typename bv_type>
void axis_slave_fifo_adapter<fifo_type, bv_type>::sc_bitvector_to_inst_fifo() {
  payload_bitvector = buffer.front();
  fifo_type fifo_mop = fifo_type();
  fifo_mop.load_from_bv(payload_bitvector);
  fifo_rdata[0].write(fifo_mop);
}

// Sequential logic of the AXI-stream slave adapter interface
template <typename fifo_type, typename bv_type>
void axis_slave_fifo_adapter<fifo_type, bv_type>::Tick() {
  // Reset logic
  buffer_occupancy.write(0);
  while (!buffer.empty()) buffer.pop();
  transfer_count.write(0);
  wait();

  while (true) {
    bool pushed_into_buffer = false;
    bool popped_from_buffer = false;

    // When a transfer happens (i.e., tvalid & tready are asserted), buffer incoming data as a part
    // of the payload depending on the value of the transfer counter
    if (axis_port.tready.read() && axis_port.tvalid.read()) {
      // Set the corresponding part of the payload to be buffered
      unsigned int start_idx = transfer_count.read() * interface_dataw;
      unsigned int end_idx = std::min((transfer_count.read() + 1) * interface_dataw, payload_bitwidth);
      buffer_wdata.range(end_idx - 1, start_idx) = axis_port.tdata.read().range(interface_dataw - 1, 0);

      // If this is the last transfer of a payload, set metadata, update transfer counter and push
      // payload to the buffer - this models the behaviour of an asymmetric FIFO if AXI-stream data
      // width is smaller than the payload data width
      if (transfer_count.read() == transfers_per_axis_packet - 1) {
        // Write ID, address, and flag as part of payload to be buffer if it's a writeback interface
        if (interface_type == MVU_WRITEBACK_INTERFACE || interface_type == VEW_WRITEBACK_INTERFACE) {
          unsigned int vrf_id_start_idx = payload_bitwidth - VRF_WB_SELW;
          unsigned int addr_start_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW;
          unsigned int flag_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW - 1;
          sc_bv<AXIS_USERW> tuser_field = axis_port.tuser.read();
          buffer_wdata.range(vrf_id_start_idx + VRF_WB_SELW - 1, vrf_id_start_idx) =
              tuser_field.range(VRF_ADDRW + VRF_WB_SELW, VRF_ADDRW + 1);
          buffer_wdata.range(addr_start_idx + VRF_ADDRW - 1, addr_start_idx) = tuser_field.range(VRF_ADDRW, 1);
          buffer_wdata.set_bit(flag_idx, tuser_field.get_bit(0));
          if (interface_type == VEW_WRITEBACK_INTERFACE) {
            unsigned int block_id_start_idx = payload_bitwidth - VRF_WB_SELW - VRF_ADDRW - 1 - BLOCK_WB_SELW;
            buffer_wdata.range(block_id_start_idx + BLOCK_WB_SELW - 1, block_id_start_idx) = axis_port.tdest.read();
          }
        }
        transfer_count.write(0);
        buffer.push(buffer_wdata);
        pushed_into_buffer = true;
      } else {
        transfer_count.write(transfer_count.read() + 1);
      }
    }

    // If the receiving FIFO is ready and payload exists in adapter buffer, pop it
    if (fifo_ren[0].read() && fifo_rdy[0].read()) {
      buffer.pop();
      popped_from_buffer = true;
    }

    // If buffer is not empty, update the FIFO read data interface with the payload at the front
    // of the buffer
    if (!buffer.empty()) {
      if (interface_type == FEEDFORWARD_INTERFACE || interface_type == VEW_WRITEBACK_INTERFACE || interface_type == MVU_WRITEBACK_INTERFACE) {
        sc_bitvector_to_data_fifo();
      } else {
        sc_bitvector_to_inst_fifo();
      }
    }

    // Adjust buffer occupancy based on what happened in this cycle
    if (pushed_into_buffer && !popped_from_buffer) {
      buffer_occupancy.write(buffer_occupancy.read() + 1);
    } else if (!pushed_into_buffer && popped_from_buffer) {
      buffer_occupancy.write(buffer_occupancy.read() - 1);
    }
    
    wait();
  }
}

// Combinational logic of the AXI-stream master adapter interface
template <typename fifo_type, typename bv_type>
void axis_slave_fifo_adapter<fifo_type, bv_type>::Assign() {
  if (rst.read()) {
    axis_port.tready.write(true);
    for (unsigned int fifo_id = 0; fifo_id < num_fifo; fifo_id++) fifo_rdy[fifo_id].write(false);
  } else {
    axis_port.tready.write(buffer_occupancy.read() < buffer_capacity);
    for (unsigned int fifo_id = 0; fifo_id < num_fifo; fifo_id++) fifo_rdy[fifo_id].write(buffer_occupancy.read() > 0);
  }
}

// Declarations of needed AXI-stream slave adapter template types
template class axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<FEEDFORWARD_DATA_WIDTH>>;
template class axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<NARROW_WRITEBACK_BV_WIDTH>>;
template class axis_slave_fifo_adapter<data_vector<tb_input_precision>, sc_bv<WIDE_WRITEBACK_BV_WIDTH>>;
template class axis_slave_fifo_adapter<data_vector<tb_output_precision>, sc_bv<WIDE_WRITEBACK_BV_WIDTH>>;
template class axis_slave_fifo_adapter<mvu_mop, sc_bv<MVU_MOP_BITWIDTH>>;
template class axis_slave_fifo_adapter<evrf_mop, sc_bv<EVRF_MOP_BITWIDTH>>;
template class axis_slave_fifo_adapter<mfu_mop, sc_bv<MFU_MOP_BITWIDTH>>;
template class axis_slave_fifo_adapter<ld_mop, sc_bv<LD_MOP_BITWIDTH>>;