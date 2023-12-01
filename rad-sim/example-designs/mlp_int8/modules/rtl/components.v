/**
Pipeline Module
Convenient module to specify register delays

Parameters:
DELAY: Number of Registers to delay data_in by
WIDTH: Width of data

Inputs:
data_in: The data to be delayed

Outputs:
data_out: The data after the pipelined delay
**/

module pipeline # (
	parameter DELAY = 1,
	parameter WIDTH = 32
)(
	input  clk,
	input  rst,
	input  [WIDTH-1:0] data_in,
	output [WIDTH-1:0] data_out
);

reg [WIDTH-1:0] r_pipeline [0:DELAY-1]; 

integer i;
always @ (posedge clk) begin
	if (rst) begin
		for (i = 0; i < DELAY; i = i + 1) begin
			r_pipeline[i] <= 0;
		end
	end else begin
		r_pipeline[0] <= data_in;
		for (i = 1; i < DELAY; i = i + 1) begin
			r_pipeline[i] <= r_pipeline[i-1];
		end
	end
end

assign data_out = r_pipeline[DELAY-1];

endmodule

/**
Memory Block Module
A dual port memory with 1 Write and 1 Read. Can read and write simultaneously if not on the same address.

Parameters:
DATAW: Width of data word
DEPTH: Depth of the memory
ADDRW: Width of memory address

Inputs:
waddr: Write Address
wen: Write Enable
wdata: Write Data
raddr: Read Address

Outputs:
rdata: Read Data
**/
module memory_block # (
	parameter DATAW = 8,
	parameter DEPTH = 512,
	parameter ADDRW = $clog2(DEPTH)
)(
	input  clk,
	input  rst,
	input  [ADDRW-1:0] waddr,
	input  wen,
	input  [DATAW-1:0] wdata,
	input  [ADDRW-1:0] raddr,
	output [DATAW-1:0] rdata
);

reg [DATAW-1:0] mem [0:DEPTH-1];

reg [ADDRW-1:0] r_raddr, r_waddr;
reg [DATAW-1:0] r_rdata, r_wdata;
reg r_wen;

integer i;

initial begin
	for (i = 0; i < DEPTH; i = i + 1) begin
		mem[i] = 0;
	end
end

always @ (posedge clk) begin
	if (rst) begin
		r_raddr <= 0;
		r_waddr <= 0;
		r_wdata <= 0;
		r_rdata <= 0;
	end else begin
		r_raddr <= raddr;
		r_wen <= wen;
		r_waddr <= waddr;
		r_wdata <= wdata;
		r_rdata <= mem[r_raddr];
		
		if (r_wen) mem[r_waddr] <= r_wdata;
	end
end

assign rdata = r_rdata;

endmodule

/**
FIFO Module
A peek FIFO implementation

Parameters:
DATAW: Width of data word
DEPTH: Depth of the FIFO
ADDRW: Width of FIFO address for pointer use
ALMOST_FULL_DEPTH: Depth of the FIFO at which the almost_full signal is asserted

Inputs:
push: Signal to push data to the FIFO
idata: Input Data
pop: Siginal to pop data from the FIFO

Outputs:
odata: Output Data
empty: Signal asserted when FIFO is empty
full: Signal asserted when FIFO is full
almost_full: Signal asserted when FIFO is almost full defined by ALMOST_FULL_DEPTH
**/
module fifo # (
	parameter DATAW = 64,
	parameter DEPTH = 128,
	parameter ADDRW = $clog2(DEPTH),
	parameter ALMOST_FULL_DEPTH = DEPTH
)(
	input  clk,
	input  rst,
	input  push,
	input  [DATAW-1:0] idata,
	input  pop,
	output [DATAW-1:0] odata,
	output empty,
	output full,
	output almost_full
);

reg [DATAW-1:0] mem [0:DEPTH-1];
reg [ADDRW-1:0] head_ptr, tail_ptr;
reg [ADDRW:0] remaining;

always @ (posedge clk) begin
	if (rst) begin
		head_ptr <= 0;
		tail_ptr <= 0;
		remaining <= DEPTH;
	end else begin
		if (!full && push) begin
			mem[tail_ptr] <= idata;
			tail_ptr <= tail_ptr + 1'b1;
		end
		
		if (!empty && pop)  begin
			head_ptr <= head_ptr + 1'b1;
		end
		
		if (!empty && pop && !full && push) begin
			remaining <= remaining;
		end else if (!empty && pop) begin
			remaining <= remaining + 1'b1;
		end else if (!full && push) begin
			remaining <= remaining - 1'b1;
		end else begin
			remaining <= remaining;
		end
	end
end

assign empty = (tail_ptr == head_ptr);
assign full = (tail_ptr + 1'b1 == head_ptr);
assign odata = mem[head_ptr];
assign almost_full = (remaining < (DEPTH - ALMOST_FULL_DEPTH));

endmodule