#include <receiver.hpp>

receiver::receiver(const sc_module_name &name, RADSimDesignContext* radsim_design)
      : RADSimModule(name, radsim_design) {
    this->radsim_design = radsim_design;
 
    sensitive << rst;

    SC_CTHREAD(Tick, clk.pos());
    reset_signal_is(rst, true); // Reset is active high

    RegisterModuleInfo();
    refFile.open("sender_output.txt");
}

receiver::~receiver() {
    //check if any data is left in the reference file
    std:string line;
    if(std::getline(refFile, line)) {
        std::cout << "Some data have not arrived!" << std::endl;
        errorFlag = true;
    }
    if (errorFlag) {
        std::cerr << "Error occurred during data reception!" << std::endl;
    } else {
        std::cout << "Data reception completed successfully!" << std::endl;
    }
    refFile.close();
}

void receiver::Tick() {
    if (rst) {
        axis_receiver_interface.tready.write(false);  
    } else {
        axis_receiver_interface.tready.write(true); // Ready to receive data
    }
    wait();
    errorFlag = false;
    while (true) {
        if (axis_receiver_interface.tvalid.read() && axis_receiver_interface.tready.read()) {
            unsigned int tdata = axis_receiver_interface.tdata.read().range(31, 0).to_uint();;
            bool tlast = axis_receiver_interface.tlast.read();
            std::cout << "Received Data: " << std::to_string(tdata) << ", Last: " << tlast << std::endl;
            //compare with reference file
            if (refFile.is_open()) {
                std::string line;
                if (std::getline(refFile, line)) {
                    if (line != std::to_string(tdata)) {
                        std::cout << "Data mismatch! Expected: " << line << ", Received: " << std::to_string(tdata) << std::endl;
                        errorFlag = true;
                    }
                } else {
                    std::cout << "No more lines in reference file." << std::endl;
                    errorFlag = true;
                }
            } else {
                std::cerr << "Reference file not open!" << std::endl;
                errorFlag = true;
            }
            
        }
        wait();
    }
}


void receiver::RegisterModuleInfo() {
    std::string port_name = module_name + ".axis_receiver_interface";
    RegisterAxisSlavePort(port_name, &axis_receiver_interface, 128, 0);
}