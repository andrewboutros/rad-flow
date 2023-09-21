// Adder Module
// George Trieu

`include "static_params.vh"
 
module adder (clk, rst, axis_adder_interface_tvalid, axis_adder_interface_tlast, axis_adder_interface_tdata, axis_adder_interface_tready);
    input clk;
    input rst;
    input axis_adder_interface_tvalid;
    input axis_adder_interface_tlast;
    input [`AXIS_MAX_DATAW-1:0] axis_adder_interface_tdata;
	 
	 output reg axis_adder_interface_tready;
 
    reg [`DATAW-1:0] adder_rolling_sum;
    reg t_finished;
 
    always @(rst) begin
        if (rst) begin
            adder_rolling_sum = {`DATAW{1'b0}};
            t_finished = 1'b0;
            axis_adder_interface_tready = 1'b0;
        end else begin
            axis_adder_interface_tready = 1'b1;
        end
    end
 
    always @(posedge clk) begin
        if (axis_adder_interface_tready && axis_adder_interface_tvalid) begin
            $display("Adder: Received %d!", axis_adder_interface_tdata[63:0]);
            adder_rolling_sum = adder_rolling_sum + axis_adder_interface_tdata[`DATAW-1:0];
            t_finished = axis_adder_interface_tlast;
        end
 
        if (t_finished) begin
            $display("The final sum of all the addends in the transactions is: %d", adder_rolling_sum[63:0]);
            $finish;
        end
    end
endmodule
