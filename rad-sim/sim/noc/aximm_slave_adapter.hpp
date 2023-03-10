#pragma once

#include <systemc.h>

#include <algorithm>
#include <aximm_interface.hpp>
#include <booksim_config.hpp>
#include <buffer_state.hpp>
#include <cmath>
#include <flit.hpp>
#include <network.hpp>
#include <queue>
#include <radsim_telemetry.hpp>
#include <radsim_utils.hpp>
#include <noc_utils.hpp>
#include <routefunc.hpp>
#include <sc_flit.hpp>

/* This class is the SystemC implementation of the AXI Memory Mapped (AXI-MM) slave NoC adapter. This adapter acts as
 * the access point to the NoC. On one side, it has a full AXI-MM interface to send read/write request transactions
 * (AR, AW, W) and receive read/write response transactions (R, B). On the other side, the adapter interfaces with the
 * Booksim NoC simulator.
 * The slave adapter has the following high-level organization:
 *
 *     B <==|‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|   |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|   |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|   |‾‾‾‾‾‾‾‾‾|
 *     R <==|               |<==| De-packetization |<==| Flit Ejection  |<==|         |
 *          | AXI Interface |   |__________________|   |________________|   | Booksim |
 *    AR ==>|     Logic     |   |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|   |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|   |   NoC   |
 *    AW ==>|               |==>|   Packetization  |==>| Flit Injection |==>|         |
 *     W ==>|_______________|   |__________________|   |________________|   |_________|
 *
 * - AXI Interface Logic: This block implements the arbitration between request interfaces (AR, AW, W) & forwards the
 *   selected transation to be packetized. It also steers the de-packetized response transaction to the right interface
 *   (B, R) based on its type. This block operates at the node frequency by default.
 *
 * - Packetization: This block processes the selected request transaction and transforms it into one or more SystemC
 *   flits (based on the flit definition in sc_flit.hpp) as long as the ratio between the AXI interface logic and the
 *   packetization frequency allows that (i.e. number of flits per transaction packet has to be less than or equal to
 *   the frequency ratio). Produced flits are inserted into an asynchronous FIFO to be later injected into the NoC.
 *   This block operates at the adapter frequency by default.
 *
 * - Flit Injection: This block pops SystemC flits from the asynchronous injection FIFO, transforms them into Booksim
 *   flits, maps them to corresponding virtual channels based on flit typen, and injects them into the NoC if possible.
 *   This module operates at the NoC frequency by default.
 *
 * - De-packetization and Flit Ejection blocks work similarly (but in reverse direction) to the Packetization and Flit
 *   injection blocks, respectively. They eject Booksim flits from the NoC when available, store them in VC buffers
 *   until full packet is received, de-packetizes them, and presents them as AXI-MM response transactions (B, R) on the
 *   adapter output ports.
 * */

class aximm_slave_adapter : public sc_module {
private:
    // The ID and reconfigurable width of the node this adapter is connected to
    unsigned int _node_id;
    double _node_period, _adapter_period, _noc_period;
    unsigned int _network_id;
    unsigned int _interface_dataw;
    unsigned int _num_flits_per_packet;
    unsigned int _freq_ratio;

    // Pointers to Booksim datastructures: NoC, buffer states, routing info, bookkeeping of ejected flits
    BookSimConfig* _noc_config;
    Network* _noc;
    BufferState* _buffer_state;
    tRoutingFunction _routing_func;
    bool _lookahead_routing;
    bool _wait_for_tail_credit;
    map<int, int>* _ejected_flits;

    /* Request interface signals for storing selected input request transaction based on the following table:
     * -------------------------------------------------------------------------
     * |           type           |   AR    |   AW    |           W            |
     * |            id            |  arid   |  awid   |          wid           |
     * | payload[MAX_DATAW:ADDRW] |         |         | wdata[MAX_DATAW:ADDRW] |
     * |    payload[ADDRW-1:0]    | araddr  | awaddr  |    wdata[ADDRW-1:0]    |
     * |        ctrl[12:5]        | arlen   | awlen   |                        |
     * |        ctrl[4:2]         | arsize  | awsize  |                        |
     * |        ctrl[1:0]         | arburst | awburst |       {X, wlast}       |
     * |          valid           | arvalid | awvalid |         wvalid         |
     * |          ready           | arready | awready |         wready         |
     * ------------------------------------------------------------------------- */
    sc_signal<sc_uint<AXI_IDW>> _i_id;
    sc_signal<sc_bv<AXI_MAX_DATAW>> _i_payload;
    sc_signal<sc_bv<AXI_CTRL>> _i_ctrl;
    sc_signal<sc_bv<AXI_USERW>> _i_user;
    sc_signal<bool> _i_valid;
    sc_signal<sc_uint<2>> _i_type;
    sc_signal<int> _i_unique_sim_id;

    // Variables for computing different request interface ready signals. Slave adapter will accept a W transaction
    // only if it previously received an AW transaction and did not receive wlast yet (specified by the _got_aw signal).
    // As in AXI4 protocol, W transactions will be directed to the last stored AW address (_last_awaddr) regardless to
    // the transaction ID.
    bool _arready, _awready, _wready;
    sc_signal<bool> _got_aw;
    sc_signal<sc_bv<AXI_ADDRW>> _last_awaddr;
    // Priority setting for AXI Interface arbitration logic. Current implementation is a prioritized round-robin with
    // order as: AR->AW->W (e.g. if AR transaction is selected, next highest priority will be AW)
    sc_signal<sc_uint<2>> _injection_priority_setting;

    // Packetization FSM variables: _packetization_state goes from (0) to (_max_flits_per_transaction-1). During each
    // adapter cycle, a flit is constructed and pushed to asynchronous injection FIFO
    int _num_flits;
    unsigned int _packetization_cycle;

    // Asynchronous injection FIFO
    int _injection_afifo_depth;
    std::queue<sc_flit> _injection_afifo;
    sc_signal<bool> _injection_afifo_full;
    sc_signal<bool> _packetization_busy;

    // Booksim flit injection variables
    int _last_vc_id;
    bool _injection_flit_ready;
    sc_flit _to_be_injected_flit;

    // Booksim flit ejection variables
    Flit* _ejected_booksim_flit;

    // Asynchronous ejection FIFOs, one for each response type (R, B)
    int _ejection_afifo_depth;
    sc_fifo<sc_flit>* _ejection_afifos [2]{};
    sc_signal<int> _ejection_afifo_push_counter [2];
    sc_signal<int> _ejection_afifo_pop_counter [2];

    // Asynchronous ejection FIFOs pop priority such that top of the queue is highest priority FIFO ID to pop from.
    // Current implementation is prioritized round-robin.
    std::queue<unsigned int> _ejection_afifo_priority;

    // Flag to signal that a packet is being depacketized
    sc_signal<bool> _ejection_afifo_is_depacketizing;

    // Double buffer for output packet, implemented as a FIFO of size 2
    sc_fifo<sc_packet>* _output_afifo;

    // Packet datastructures for packets depacketized and pushed (_constructed_packet)/popped (_output_packet) to/from
    // the output double-buffer
    sc_packet _constructed_packet, _output_packet;

    // Flag to signal an output packet is already popped from output FIFO and waiting for ouptut interface to be ready
    bool _output_packet_ready;

public:
    // Clocks and reset
    sc_in<bool> node_clk;
    sc_in<bool> adapter_clk;
    sc_in<bool> noc_clk;
    sc_in<bool> rst;
    // AXI-MM Slave Port
    aximm_slave_port aximm_interface;

    aximm_slave_adapter(const sc_module_name& name, int node_id, int network_id,
                        unsigned int interface_dataw, double node_period, double adapter_period,
                        BookSimConfig *noc_config, Network *noc, BufferState *buffer_state, 
                        tRoutingFunction routing_func, bool lookahead_routing, 
                        bool wait_for_tail_credit, map<int, int> *ejected_flits);
    ~aximm_slave_adapter() override;

    // SystemC Threads implementing stages of input pipeline of the adapter
    void InputReady();
    void InputInterface();
    void InputPacketization();
    void InputInjection();

    // SystemC Threads implementing stages of output pipeline of the adapter
    void OutputInterface();
    void OutputDepacketization();
    void OutputEjection();

    // Helper functions
    static sc_bv<AXI_DESTW> InputAddressTranslation(sc_bv<AXI_ADDRW>& addr);
    int VCMapping(sc_flit& flit);
    int InverseVCMapping(int vc_id);

    SC_HAS_PROCESS(aximm_slave_adapter);
};