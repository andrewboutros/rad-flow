#include "simplified_dpe.hpp"

simplified_dpe::simplified_dpe(const sc_module_name& name, unsigned int id, unsigned int tile_id) :
        sc_module(name),
        vector_a("vector_a"),
        valid_a("valid_a"),
        vector_b("vector_b"),
        reg_shift_sel("reg_shift_sel"),
        reg_use_sel("reg_use_sel"),
        result("result"),
        valid_result("valid_result"),
        vector_b_delay("vector_b_delay", TB_NUM_DOTS),
        valid_b_delay("valid_b_delay", TB_NUM_DOTS+1),
        reg_shift_sel_delay("reg_shift_sel_delay", TB_NUM_DOTS+1),
        result_delay("result_delay", DPE_VALID_A_PIPELINE),
        result_valid_delay("result_valid_delay", DPE_VALID_A_PIPELINE) {

    // Set member variables
    dpe_id = id;
    dpe_tile_id = tile_id;

    // Size the tensor blocks register banks
    tb_regs.resize( 2 );
    for( unsigned int t = 0; t < 2; t++ ) {
        tb_regs[t].resize(TB_NUM_DOTS);
        for (unsigned int dot_id = 0; dot_id < TB_NUM_DOTS; dot_id++) {
            tb_regs[t][dot_id].resize(LANES);
        }
    }
    batch_counter = 0; tb_counter = 0;

    SC_METHOD( AssignSignals );
    sensitive << rst << result_valid_delay[DPE_VALID_A_PIPELINE-1] << result_delay[DPE_VALID_A_PIPELINE-1];
    SC_CTHREAD( AdvanceControlPipelines, clk.pos() );
    reset_signal_is( rst, true );
}

simplified_dpe::~simplified_dpe() { }

void simplified_dpe::AdvanceControlPipelines(){
    // Reset logic
    for( unsigned int i = 0; i < TB_NUM_DOTS+1; i++ ) {
        reg_shift_sel_delay[i].write( false );
        valid_b_delay[i].write( false );
    }
    for( unsigned int i = 0; i < DPE_VALID_A_PIPELINE; i++ ){
        result_valid_delay[0].write( false );
    }
    wait();

    while( true ){
        // Advance pipeline
        if (!stall.read()){
            valid_b_delay[0].write( valid_b.read() );
            reg_shift_sel_delay[0].write( reg_shift_sel.read() );
            vector_b_delay[0].write( data_vector<tb_input_precision>(vector_b) );
            for( unsigned int i = 1; i < TB_NUM_DOTS+1; i++ ) {
                reg_shift_sel_delay[i].write( reg_shift_sel_delay[i-1] );
                valid_b_delay[i].write( valid_b_delay[i-1] );
            }
            for( unsigned int i = 1; i < TB_NUM_DOTS; i++ ) {
                vector_b_delay[i].write( vector_b_delay[i-1] );
            }

            if( valid_b_delay[TB_NUM_DOTS-1].read() ){
                data_vector<tb_input_precision> temp = vector_b_delay[TB_NUM_DOTS-1].read();
                for( unsigned int j = 0; j < TB_LANES; j++ ) {
                    tb_regs[reg_shift_sel_delay[TB_NUM_DOTS-1].read()][batch_counter][
                            (tb_counter * TB_LANES) + j] = temp[j];
                }
                if( batch_counter == TB_NUM_DOTS-1 ){
                    batch_counter = 0;
                    if( tb_counter == DPE_NUM_TBS-2 ){
                        tb_counter = 0;
                    } else {
                        tb_counter++;
                    }
                } else {
                    batch_counter++;
                }

                //if (dpe_tile_id == 0 && dpe_id == 0)
                //    cerr << reg_shift_sel_delay[TB_NUM_DOTS-1].read() << " -- "
                //        << batch_counter << " -- " << temp << endl;
            }

            result_valid_delay[0].write( valid_a.read() );
            if( valid_a.read() ){
                data_vector<tb_output_precision> res = data_vector<tb_output_precision>(TB_NUM_DOTS);
                data_vector<tb_input_precision> vec_a = vector_a.read();
                for( unsigned int dot_id = 0; dot_id < TB_NUM_DOTS; dot_id++ ){
                    res[dot_id] = 0;
                    for( unsigned int lane_id = 0; lane_id < LANES; lane_id++ ){
                        res[dot_id] += vec_a[lane_id] * tb_regs[reg_use_sel.read()][dot_id][lane_id];
                    }
                }
                result_delay[0].write( res );
            }
            for( unsigned int i = 1; i < DPE_VALID_A_PIPELINE; i++ ){
                result_valid_delay[i].write( result_valid_delay[i-1] );
                result_delay[i].write( result_delay[i-1].read() );
            }
        } 

        wait();
    }
}

void simplified_dpe::AssignSignals(){
    data_vector<tb_output_precision> res = result_delay[DPE_VALID_A_PIPELINE-1].read();
    if( res.size() != 0 ) {
        result.write( result_delay[DPE_VALID_A_PIPELINE-1].read() );
    }
    valid_result.write( result_valid_delay[DPE_VALID_A_PIPELINE-1].read() );
}