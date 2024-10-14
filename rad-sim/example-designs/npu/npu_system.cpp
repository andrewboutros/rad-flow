#include <npu_system.hpp>

npu_system::npu_system(const sc_module_name &name, sc_clock* driver_clk_sig, RADSimDesignContext* radsim_design)
    : sc_module(name),
      inst_wdata("inst_wdata"),
      inst_waddr("inst_waddr"),
      inst_wen("inst_wen"),
      start_pc("start_pc"),
      end_pc("end_pc"),
      start("start"),
      mrf_waddr("mrf_waddr"),
      mrf_wdata("mrf_wdata"),
      mrf_wid("mrf_wid"),
      ififo_rdy("ififo_rdy"),
      ififo_wen("ififo_wen"),
      ififo_wdata("ififo_wdata"),
      ofifo_rdy("ofifo_rdy"),
      ofifo_ren("ofifo_ren"),
      ofifo_rdata("ofifo_rdata"),
      rst_sig("rst_sig") {

  init_vector<sc_signal<bool>>::init_sc_vector(ififo_rdy, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ififo_wen, THREADS, CORES);
  init_vector<sc_signal<data_vector<tb_input_precision>>>::init_sc_vector(ififo_wdata, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ofifo_rdy, THREADS, CORES);
  init_vector<sc_signal<bool>>::init_sc_vector(ofifo_ren, THREADS, CORES);
  init_vector<sc_signal<data_vector<tb_output_precision>>>::init_sc_vector(ofifo_rdata, THREADS, CORES);

  npu_driver_inst = new npu_driver("npu_driver_inst", radsim_design);
  npu_driver_inst->clk(*driver_clk_sig);
  npu_driver_inst->rst(rst_sig);
  npu_driver_inst->inst_wdata(inst_wdata);
  npu_driver_inst->inst_waddr(inst_waddr);
  npu_driver_inst->inst_wen(inst_wen);
  npu_driver_inst->start_pc(start_pc);
  npu_driver_inst->end_pc(end_pc);
  npu_driver_inst->start(start);
  npu_driver_inst->mrf_waddr(mrf_waddr);
  npu_driver_inst->mrf_wdata(mrf_wdata);
  npu_driver_inst->mrf_wid(mrf_wid);
  npu_driver_inst->ififo_rdy(ififo_rdy);
  npu_driver_inst->ififo_wen(ififo_wen);
  npu_driver_inst->ififo_wdata(ififo_wdata);
  npu_driver_inst->ofifo_rdy(ofifo_rdy);
  npu_driver_inst->ofifo_ren(ofifo_ren);
  npu_driver_inst->ofifo_rdata(ofifo_rdata);

  npu_inst = new npu_top("npu_inst", radsim_design);
  npu_inst->rst(rst_sig);
  npu_inst->inst_wdata(inst_wdata);
  npu_inst->inst_waddr(inst_waddr);
  npu_inst->inst_wen(inst_wen);
  npu_inst->start_pc(start_pc);
  npu_inst->end_pc(end_pc);
  npu_inst->start(start);
  npu_inst->mrf_waddr(mrf_waddr);
  npu_inst->mrf_wdata(mrf_wdata);
  npu_inst->mrf_wid(mrf_wid);
  npu_inst->ififo_rdy(ififo_rdy);
  npu_inst->ififo_wen(ififo_wen);
  npu_inst->ififo_wdata(ififo_wdata);
  npu_inst->ofifo_rdy(ofifo_rdy);
  npu_inst->ofifo_ren(ofifo_ren);
  npu_inst->ofifo_rdata(ofifo_rdata);

  //add _top as dut instance for parent class RADSimDesignSystem
  this->design_dut_inst = npu_inst;
}

npu_system::~npu_system() {
  delete npu_driver_inst;
  delete npu_inst;
}
