#pragma once

#include <systemc.h>
#include <sim_utils.hpp>
#include <vector>
#include <radsim_telemetry.hpp>
#include "config.hpp"
#include "pipeline.hpp"

class datapath : public sc_module {
 private:
  unsigned int layer_id;
  unsigned int mvm_id;
  unsigned int datapath_id;
  std::vector<sc_int<OPRECISION>> accum_mem;
  pipeline<sc_int<OPRECISION>>* datapath_pipeline_data;
  sc_signal<sc_int<OPRECISION>> datapath_pipeline_idata, datapath_pipeline_odata;
  pipeline<bool>* datapath_pipeline_valid;
  sc_signal<bool> datapath_pipeline_ivalid, datapath_pipeline_ovalid;

 public:
  sc_in<bool> clk;
  sc_in<bool> rst;
  sc_in<bool> ivalid;
  sc_in<data_vector<sc_int<IPRECISION>>> dataa; // input operand
  sc_in<data_vector<sc_int<IPRECISION>>> datab; // rf operand
  sc_in<sc_int<IPRECISION>> datac;              // reduction operand
  sc_in<uint32_t> accum_addr;
  sc_in<bool> accum;
  sc_in<bool> last;
  sc_in<bool> reduce;
  sc_out<bool> ovalid;
  sc_out<sc_int<OPRECISION>> oresult;

  datapath(const sc_module_name& name, unsigned int id_layer, unsigned int id_mvm, unsigned int id_datapath);
  ~datapath();

  void Tick();
  void Assign();
  SC_HAS_PROCESS(datapath);
};