// Adder Module
// George Trieu

`include "static_params.vh"
 
module adder (clk, rst, axis_adder_tvalid, axis_adder_tlast, axis_adder_tdata, axis_adder_tready, response, response_valid);
    input clk;
    input rst;
    input axis_adder_tvalid;
    input axis_adder_tlast;
    input [`AXIS_MAX_DATAW-1:0] axis_adder_tdata;
	 
	 output reg axis_adder_tready;
     output reg [`DATAW-1:0] response;
     output reg response_valid;
 
    reg [`DATAW-1:0] adder_rolling_sum;
    reg t_finished;
 
    always @(rst) begin
        if (rst) begin
            adder_rolling_sum = {`DATAW{1'b0}};
            t_finished = 1'b0;
            axis_adder_tready = 1'b0;
        end else begin
            axis_adder_tready = 1'b1;
        end
    end
 
    always @(posedge clk) begin
        if (rst) begin
            response = {`DATAW{1'b0}};
            response_valid = 1'b0;
        end else begin
            if (axis_adder_tready && axis_adder_tvalid) begin
                //$display("Adder: Received %d!", axis_adder_tdata[63:0]);
                adder_rolling_sum = adder_rolling_sum + axis_adder_tdata[`DATAW-1:0];
                t_finished = axis_adder_tlast;
            end
    
            if (t_finished) begin
                response = adder_rolling_sum;
                response_valid = 1'b1;
            end
        end
    end
endmodule
