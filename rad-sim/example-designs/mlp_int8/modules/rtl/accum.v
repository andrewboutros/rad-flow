/**
Accumulator Module
Responsible for accumulating sums from previous subset vectors the MVM processed as part of one input vector. Independent of each DPE.

Inputs:
i_valid: The data in is valid
i_data: The data
i_addr: The address in the accum_mem to write/read to
i_last: The last subset in the input vector. Required for o_valid to be propagated as true.

Outputs:
o_valid: The result out is valid
o_result: The result
**/

module accum # (
	parameter DATAW = 32,
	parameter DEPTH = 512,
	parameter ADDRW = $clog2(DEPTH)
)(
	input              clk,
	input              rst,
	input              i_valid,
	input  [DATAW-1:0] i_data,
	input  [ADDRW-1:0] i_addr,
	input              i_accum,
	input              i_last,
	output             o_valid,
	output [DATAW-1:0] o_result
);

reg [ADDRW-1:0] accum_mem_waddr;
reg [DATAW-1:0] accum_mem_wdata;
wire [DATAW-1:0] accum_mem_rdata;
reg accum_mem_wen;

reg r_accum, rr_accum, r_valid, rr_valid, rrr_valid, r_last, rr_last;
reg [ADDRW-1:0] r_addr, rr_addr;
reg [DATAW-1:0] r_data, rr_data, r_result;

memory_block # (
	.DATAW(DATAW),
	.DEPTH(DEPTH)
) accum_mem (
	.clk(clk),
	.rst(rst),
	.waddr(accum_mem_waddr),
	.wen(accum_mem_wen),
	.wdata(r_result),
	.raddr(i_addr),
	.rdata(accum_mem_rdata)
);

always @ (posedge clk) begin
	if (rst) begin
		r_accum <= 1'b0; rr_accum <= 1'b0;
		r_valid <= 1'b0; rr_valid <= 1'b0; rrr_valid <= 1'b0;
		r_addr <= 'd0; rr_addr <= 'd0;
		r_data <= 'd0; rr_data <= 'd0;
		r_last <= 1'b0; rr_last <= 1'b0;
		accum_mem_wen <= 1'b0;
		accum_mem_waddr <= 'd0;
	end else begin
		// Pipeline inputs to align with memory output
		r_accum  <= i_accum;
		rr_accum <= r_accum;
		r_addr   <= i_addr;
		rr_addr  <= r_addr;
		r_valid  <= i_valid;
		rr_valid <= r_valid;
		r_last   <= i_last;
		rr_last  <= r_last;
		r_data   <= i_data;
		rr_data  <= r_data;
		
		// Perform accumulation
		if (rr_valid && rr_accum) begin
			r_result <= rr_data + accum_mem_rdata;
			accum_mem_wen <= 1'b1;
		end else if (rr_valid) begin
			r_result <= rr_data;
			accum_mem_wen <= 1'b1;
		end else begin
			accum_mem_wen <= 1'b0;
		end
		accum_mem_waddr <= rr_addr;
		rrr_valid <= rr_last && rr_valid;
	end
end

assign o_valid = rrr_valid;
assign o_result = r_result;

endmodule