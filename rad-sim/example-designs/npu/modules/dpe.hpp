#pragma once

#include <systemc.h>

#include "params.hpp"
#include "sim_utils.hpp"

/*
 * This is the dot product engine (DPE) class. Depending on the number of LANES, the DPE consists of a systolic chain
 * of N tensor blocks, each implementing TB_LANES of the dot product. An additional tensor block is added to the
 * begining of the chain to be used as an input bypass to the rest of the chain. This module performs TB_NUM_DOTS dot
 * product operations as follows: res0 = A * B0, res1 = A * B1, res2 = A * B2
 *
 * The DPE has the following inputs:
 * - clk: clock signal
 * - rst: active-high reset signal
 * - vector_a: first input vector of size LANES (to be broadcast/shared across different dots inside tensor block). It
 *   is internally split into N chunks of size TB_LANES and supplied to the tensor blocks (1, ..., N) in the chain.
 *   Balancing for this input vector (to match systolic nature of the DPE is done internally using delay registers.
 * - valid_a: valid signal for first input vector
 * - vector_b: second input vector of size TB_LANES (to be loaded sequentially to the data reuse register banks of the
 *   tensor block), which is supplied to tensor block 0 (input bypass) in the DPE chain.
 * - valid_b: valid signal for second input vector
 * - reg_shift_sel: select which data reuse register bank to shift new inputs into
 * - reg_use_sel: select which data reuse register bank to use for multiplication
 *
 * The DPE has the following outputs:
 * - result: TB_NUM_DOTS distinct results for the TB_NUM_DOTS dot products inside the DPE
 * - valid_result: valid signal for the results of the DPE
 */

class dpe : public sc_module {
private:
    // Numerical ID for the DPE and its tile, can be useful in debugging
    unsigned int dpe_id;
    unsigned int dpe_tile_id;

    sc_vector <sc_signal<data_vector<tb_input_precision>>> vector_b_delay;
    sc_vector <sc_signal<bool>> valid_b_delay;
    sc_vector <sc_signal<bool>> reg_shift_sel_delay;
    sc_vector <sc_signal<data_vector<tb_output_precision>>> result_delay;
    sc_vector <sc_signal<bool>> result_valid_delay;

    std::vector <std::vector<std::vector<tb_input_precision>>> tb_regs;
    unsigned int batch_counter, tb_counter;

public:
    // DPE inputs
    sc_in <bool> clk;
    sc_in <bool> rst;
    sc_in <data_vector<tb_input_precision>> vector_a;
    sc_in <bool> valid_a;
    sc_in <data_vector<tb_input_precision>> vector_b;
    sc_in <bool> valid_b;
    sc_in <bool> reg_shift_sel;
    sc_in <bool> reg_use_sel;
    sc_in <bool> stall;

    // DPE outputs
    sc_out <data_vector<tb_output_precision>> result;
    sc_out <bool> valid_result;

    // DPE constructor and destructor
    dpe(const sc_module_name& name, unsigned int id, unsigned int tile_id);
    ~dpe();

    // Equivalent to a combinational always block for assigning signal values
    void AssignSignals();
    // Equivalent to a sequential always block for advancing all input/control pipelines in the DPE
    void AdvanceControlPipelines();

    SC_HAS_PROCESS(dpe);
};

