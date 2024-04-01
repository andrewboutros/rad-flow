module memory # (
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
		
		// Data Forwarding
		if (r_wen && r_raddr == r_waddr) begin
			r_rdata <= r_wdata;
		end else begin
			r_rdata <= mem[r_raddr];
		end
		
		if (r_wen) mem[r_waddr] <= r_wdata;
	end
end

assign rdata = r_rdata;

endmodule