// Synchronous FIFO Module
// George Trieu
// Usage:
//  To write, pull w_enable HIGH and have data on the data_in bus
//  To read and pop, pull r_enable HIGH
//  To read and not pop (i.e. peek), pull r_enable LOW
// Behaviour:
//  If the FIFO is full (fifo_full == 1), any successive writes will not occur
//  If the FIFO is empty (fifo_empty == 1), any successive pops will not occur, and the read will produce unknown

module fifo #(parameter DEPTH=8, DATA_WIDTH=8) (
    input clk,
    input rst,
    input w_enable,
    input r_enable,
    input [DATA_WIDTH - 1:0] data_in,
    output [DATA_WIDTH - 1:0] data_out,
    output full,
    output empty
);

reg [$clog2(DEPTH) - 1:0] r_ptr;
reg [$clog2(DEPTH) - 1:0] w_ptr;
reg [DATA_WIDTH - 1:0] fifo[0:DEPTH-1];
reg [$clog2(DEPTH):0] count;

assign full = count == DEPTH;
assign empty = count == 0;
assign data_out = fifo[r_ptr];

integer i;

always @(posedge clk) begin
	if (rst) begin
		r_ptr <= 0;
		w_ptr <= 0;
		count <= 0;
		for (i=0; i < DEPTH; i=i+1) begin
			fifo[i] <= 0;
		end
	end else begin
		if (w_enable && r_enable) begin
			if (~full) begin
				fifo[w_ptr] <= data_in;
				w_ptr <= w_ptr + 1;
				if (w_ptr == DEPTH) begin
					w_ptr <= 0;
				end
				if (empty) begin
					count <= count + 1; 
				end
			end
			if (~empty) begin
				r_ptr <= r_ptr + 1;
				if (r_ptr == DEPTH) begin
					r_ptr <= 0;
				end
				if (full) begin
					count <= count - 1; 
				end
			end
	 	end else if (w_enable && ~full) begin
			fifo[w_ptr] <= data_in;
			w_ptr <= w_ptr + 1;
			if (w_ptr == DEPTH) begin
				w_ptr <= 0;
			end
			count <= count + 1;
		end else if (r_enable && ~empty) begin
			//data_out <= fifo[r_ptr];
			r_ptr <= r_ptr + 1;
			if (r_ptr == DEPTH) begin
				r_ptr <= 0;
			end
			count <= count - 1;
		end
	end
end

endmodule
