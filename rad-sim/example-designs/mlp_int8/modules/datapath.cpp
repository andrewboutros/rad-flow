#include "datapath.hpp"

datapath::datapath(const sc_module_name& name, unsigned int id_layer, unsigned int id_mvm, unsigned int id_datapath)
  : sc_module(name),
    rst("rst"),
    ivalid("ivalid"),
    dataa("dataa"),
    datab("datab"),
    datac("datac"),
    accum_addr("accum_addr"),
    accum("accum"),
    last("last"),
    reduce("reduce"),
    ovalid("ovalid"),
    oresult("oresult") {

  accum_mem.resize(RF_DEPTH);
  layer_id = id_layer;
  mvm_id = id_mvm;
  datapath_id = id_datapath;

  char pipeline_name[50];
  std::string pipeline_name_str;

  pipeline_name_str = "datapath_pipeline_data";
  std::strcpy(pipeline_name, pipeline_name_str.c_str());
  datapath_pipeline_data = new pipeline<sc_int<OPRECISION>>(pipeline_name, DATAPATH_DELAY-1);
  datapath_pipeline_data->clk(clk);
  datapath_pipeline_data->rst(rst);
  datapath_pipeline_data->idata(datapath_pipeline_idata);
  datapath_pipeline_data->odata(datapath_pipeline_odata);

  pipeline_name_str = "datapath_pipeline_valid";
  std::strcpy(pipeline_name, pipeline_name_str.c_str());
  datapath_pipeline_valid = new pipeline<bool>(pipeline_name, DATAPATH_DELAY-1);
  datapath_pipeline_valid->clk(clk);
  datapath_pipeline_valid->rst(rst);
  datapath_pipeline_valid->idata(datapath_pipeline_ivalid);
  datapath_pipeline_valid->odata(datapath_pipeline_ovalid);

  SC_METHOD(Assign);
  sensitive << rst << datapath_pipeline_odata << datapath_pipeline_ovalid;
  SC_CTHREAD(Tick, clk.pos());
  reset_signal_is(rst, true);
}

datapath::~datapath() {
  delete datapath_pipeline_data;
  delete datapath_pipeline_valid;
}

void datapath::Tick() {
  // Reset logic
  datapath_pipeline_idata.write(0);
  datapath_pipeline_ivalid.write(false);
  wait();

  // Sequential logic
  while(true) {
    if (ivalid.read()) {
      sc_int<OPRECISION> dot_result = 0;
      data_vector<sc_int<IPRECISION>> dataa_operand = dataa.read();
      data_vector<sc_int<IPRECISION>> datab_operand = datab.read();
      sc_int<OPRECISION> datac_operand = datac.read();
      
      // Dot product
      for (unsigned int i = 0; i < LANES; i++) {
        dot_result += (dataa_operand[i] * datab_operand[i]);
      }

      // Reduction
      if (reduce.read()) {
        dot_result += datac_operand;
      }

      // Accumulation
      sc_int<OPRECISION> accum_mem_operand = accum_mem[accum_addr.read()];
      if (accum.read()) {
        accum_mem[accum_addr.read()] += dot_result;
      } else {
        accum_mem[accum_addr.read()] = dot_result;
      }

      /*if (layer_id == 0 && mvm_id == 1 && datapath_id == 0) {
        std::cout << "A: " << dataa_operand << std::endl;
        std::cout << "B: " << datab_operand << std::endl;
        std::cout << "C: " << datac_operand << std::endl;
      }*/

      // Produce output
      if (last.read()) {
        datapath_pipeline_idata.write(accum_mem[accum_addr.read()]);
        datapath_pipeline_ivalid.write(true);
      } else {
        datapath_pipeline_idata.write(accum_mem[accum_addr.read()]);
        datapath_pipeline_ivalid.write(false);
      }
    } else {
      datapath_pipeline_idata.write(accum_mem[accum_addr.read()]);
      datapath_pipeline_ivalid.write(false);
    }
    wait();
  }
}

void datapath::Assign() {
  if (rst) {
    ovalid.write(false);
    oresult.write(0);
  } else {
    ovalid.write(datapath_pipeline_ovalid);
    oresult.write(datapath_pipeline_odata);
  }
}