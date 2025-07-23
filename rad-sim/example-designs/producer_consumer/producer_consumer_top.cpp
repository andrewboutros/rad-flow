#include <producer_consumer_top.hpp>

producer_consumer_top::producer_consumer_top(const sc_module_name &name, RADSimDesignContext* radsim_design)
    : RADSimDesignTop(radsim_design) {

  this->radsim_design = radsim_design;

  std::string module_name_str;
  char module_name[25];

  module_name_str = "sender_inst";
  std::strcpy(module_name, module_name_str.c_str());
  int amount_to_send=5;

  sender_inst = new sender(module_name, radsim_design, amount_to_send);
  sender_inst->rst(rst);

  module_name_str = "receiver_inst";
  std::strcpy(module_name, module_name_str.c_str());

  receiver_inst = new receiver(module_name, radsim_design);
  receiver_inst->rst(rst);

  this->connectPortalReset(&rst);
  radsim_design->BuildDesignContext("producer_consumer.place", "producer_consumer.clks");
  radsim_design->CreateSystemNoCs(rst);
  radsim_design->ConnectModulesToNoC();
  start_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));
}

producer_consumer_top::~producer_consumer_top() {
  end_cycle = GetSimulationCycle(radsim_config.GetDoubleKnobShared("sim_driver_period"));
  NoCTransactionTelemetry::DumpStatsToFile("stats.csv");
  NoCFlitTelemetry::DumpNoCFlitTracesToFile("flit_traces.csv");

  std::vector<double> aggregate_bandwidths = NoCTransactionTelemetry::DumpTrafficFlows("traffic_flows", 
    end_cycle - start_cycle, radsim_design->GetNodeModuleNames(), radsim_design->rad_id);
  std::cout << "Aggregate NoC BW = " << aggregate_bandwidths[0] / 1000000000 << " Gbps" << std::endl;

  delete sender_inst;
  delete receiver_inst;
}