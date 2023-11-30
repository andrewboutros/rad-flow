/**
Reduction Module
Reduce vectors from one MVM to another

Parameters:
IPREC: Precision of elements in reduction vector
OPREC: Precision of elements in output vector

Inputs:
i_valid: Valid signal for all data
i_dataa: Vector data from host MVM
i_datab: Reduction vector from other MVM on same layer
i_reduce: Enable signal for reduction

Outputs:
o_valid: Valid signal for output result
o_result: Result
**/
module reduce # (
	parameter IPREC = 8,
	parameter OPREC = 32
)(
	input              clk,
	input              rst,
	input              i_valid,
	input  [OPREC-1:0] i_dataa,
	input  [IPREC-1:0] i_datab,
	input              i_reduce,
	output             o_valid,
	output [OPREC-1:0] o_result
);

reg [OPREC-1:0] r_result;
reg r_valid;

always @ (posedge clk) begin
	if (rst) begin
		r_result <= 'd0;
		r_valid <= 1'b0;
	end else begin
		if (i_valid && i_reduce) begin
			r_result <= i_dataa + i_datab;
			r_valid <= 1'b1;
		end else if (i_valid) begin
			r_result <= i_dataa;
			r_valid <= 1'b1;
		end else begin
			r_valid <= 1'b0;
		end
	end
end

assign o_valid = r_valid;
assign o_result = r_result;

endmodule