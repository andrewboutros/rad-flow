#include <hbm_traffic_system.hpp>

hbm_traffic_system::hbm_traffic_system(const sc_module_name &name, sc_clock *driver_clk_sig)
    : sc_module(name) {
    // Init sc_vectors ??
    mem_req_valids_sig.init(1);
    mem_req_readys_sig.init(1);
    wr_ens_sig.init(1);
    // Instantiate design top-level
    dut_inst = new hbm_traffic_top("dut");
    dut_inst->rst(rst_sig);
    /*
        dut_inst->lookup_indecies_data(lookup_indecies_data_sig);
        dut_inst->lookup_indecies_target_channels(
            lookup_indecies_target_channels_sig);
        dut_inst->lookup_indecies_base_addresses(lookup_indecies_base_addresses_sig);
        dut_inst->lookup_indecies_valid(lookup_indecies_valid_sig);
        dut_inst->lookup_indecies_ready(lookup_indecies_ready_sig);
    */
    // Traffic Gen
    dut_inst->mem_req_valids(mem_req_valids_sig);
    dut_inst->mem_req_readys(mem_req_readys_sig);
    dut_inst->target_channels(target_channels_sig);
    dut_inst->target_addresses(target_addresses_sig);
    dut_inst->wr_ens(wr_ens_sig);
    dut_inst->wr_datas(wr_datas_sig);
    dut_inst->src_ports(src_ports_sig);
    dut_inst->dst_ports(dst_ports_sig);
    // Verification signals
    dut_inst->wr_req_data(wr_req_data_sig);
    dut_inst->wr_req_data_rdy(wr_req_data_rdy_sig);
    dut_inst->rd_req_data(rd_req_data_sig);
    dut_inst->rd_req_data_rdy(rd_req_data_rdy_sig);

    // for (int ch = 0; ch < 18; ch++){
    //   dut_inst->mem_req_valid[ch](mem_req_valid_sig[ch]);
    //   dut_inst->mem_req_ready[ch](mem_req_ready_sig[ch]);
    //   dut_inst->target_channel[ch](target_channel_sig[ch]);
    //   dut_inst->target_address[ch](target_address_sig[ch]);
    //   dut_inst->write_en[ch](write_en_sig[ch]);
    // }
    // dut_inst->received_responses(received_responses_sig);
    dut_inst->collector_fifo_rdy(collector_fifo_rdy_sig);
    dut_inst->collector_fifo_ren(collector_fifo_ren_sig);
    dut_inst->collector_fifo_rdata(collector_fifo_rdata_sig);


    // Instantiate driver
    driver_inst = new hbm_traffic_driver("driver");
    driver_inst->clk(*driver_clk_sig);
    driver_inst->rst(rst_sig);
    /*
        driver_inst->lookup_indecies_data(lookup_indecies_data_sig);
        driver_inst->lookup_indecies_target_channels(
            lookup_indecies_target_channels_sig);
        driver_inst->lookup_indecies_base_addresses(
            lookup_indecies_base_addresses_sig);
        driver_inst->lookup_indecies_valid(lookup_indecies_valid_sig);
        driver_inst->lookup_indecies_ready(lookup_indecies_ready_sig);
    */
    // driver_inst->received_responses(received_responses_sig);
    driver_inst->collector_fifo_rdy(collector_fifo_rdy_sig);
    driver_inst->collector_fifo_ren(collector_fifo_ren_sig);
    driver_inst->collector_fifo_rdata(collector_fifo_rdata_sig);
    // Traffic Gen
    driver_inst->mem_req_valids(mem_req_valids_sig);
    driver_inst->mem_req_readys(mem_req_readys_sig);
    driver_inst->target_channels(target_channels_sig);
    driver_inst->target_addresses(target_addresses_sig);
    driver_inst->wr_datas(wr_datas_sig);
    driver_inst->wr_ens(wr_ens_sig);
    driver_inst->src_ports(src_ports_sig);
    driver_inst->dst_ports(dst_ports_sig);
    // Verif Signals
    driver_inst->wr_req_data(wr_req_data_sig);
    driver_inst->wr_req_data_rdy(wr_req_data_rdy_sig);
    driver_inst->rd_req_data(rd_req_data_sig);
    driver_inst->rd_req_data_rdy(rd_req_data_rdy_sig);
}

hbm_traffic_system::~hbm_traffic_system() {
  delete driver_inst;
  delete dut_inst;
}