#pragma once

#include <radsim_config.hpp>
#include <sender.hpp>
#include <receiver.hpp>
#include <systemc.h>
#include <vector>
#include <design_top.hpp>
#include <axis_interface.hpp>

#include "radsim_config.hpp"

class producer_consumer_top : public RADSimDesignTop {
private:
    sender *sender_inst;
    receiver *receiver_inst;
    RADSimDesignContext* radsim_design;
    int start_cycle;
    int end_cycle;
public:
    sc_in<bool> rst;
    producer_consumer_top(const sc_module_name &name, RADSimDesignContext* radsim_design);
    ~producer_consumer_top();
};