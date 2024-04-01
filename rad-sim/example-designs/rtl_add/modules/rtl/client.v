// Client Module
// George Trieu

`include "static_params.vh"

`define DEST_ADDR `AXIS_DESTW'b0
`define SRC_ADDR `AXIS_USERW'b11

module client (
	input clk,
	input rst,
	input [`DATAW-1:0] client_tdata,
	input client_tlast,
	input client_valid,
	input axis_client_tready,
	output client_ready,
	output axis_client_tvalid,
	output axis_client_tlast,
	output [`AXIS_DESTW-1:0] axis_client_tdest,
	output [`AXIS_IDW-1:0] axis_client_tid,
	output [`AXIS_STRBW-1:0] axis_client_tstrb,
	output [`AXIS_KEEPW-1:0] axis_client_tkeep,
	output [`AXIS_USERW-1:0] axis_client_tuser,
	output [`AXIS_MAX_DATAW-1:0] axis_client_tdata
);

	 wire fifo_w_en, fifo_r_en;
	 wire [`DATAW-1:0] fifo_data_in;
	 
	 wire [`DATAW-1:0] fifo_data_out;
	 wire fifo_full;
	 wire fifo_empty;
	 
	 reg last_item_latch;
	 integer item_count;
	 
	 // there is 2 clock cycle delays from the client receiving a LAST flag to when it is 
	  
	 fifo #(.DATA_WIDTH(`DATAW), .DEPTH(`FIFO_DEPTH)) client_tdata_fifo(
		.clk(clk),
		.rst(rst),
		.w_enable(fifo_w_en),
		.r_enable(fifo_r_en),
		.data_in(fifo_data_in),
		.data_out(fifo_data_out),
		.full(fifo_full),
		.empty(fifo_empty)
	 );
	 
	 assign client_ready = ~fifo_full;
	 assign fifo_r_en = axis_client_tvalid && axis_client_tready;
	 assign fifo_w_en = client_ready && client_valid;
	 assign fifo_data_in = client_tdata;
	 
	 assign axis_client_tdest = `DEST_ADDR;
	 assign axis_client_tuser = `SRC_ADDR;
	 assign axis_client_tid = {`AXIS_IDW{1'b0}};
	 assign axis_client_tstrb = {`AXIS_STRBW{1'b0}};
	 assign axis_client_tkeep = {`AXIS_KEEPW{1'b0}};
	 assign axis_client_tvalid = ~fifo_empty;
	 assign axis_client_tdata = fifo_data_out;
	 assign axis_client_tlast = last_item_latch && item_count == 1;
	 
    always @(posedge clk) begin
        if (rst) begin
            item_count <= 0;
				last_item_latch <= 1'b0;
        end else begin
				if (client_ready && client_valid && axis_client_tvalid && axis_client_tready) begin
					// push data onto the FIFO
					//$display("Client: Added %d onto the FIFO!", client_tdata[63:0]);
					//$display("Client: Sent %d to Adder!", axis_client_tdata[63:0]);
				end else if (client_ready && client_valid) begin
					// push data onto the FIFO
					item_count <= item_count + 1;
					//$display("Client: Added %d onto the FIFO!", client_tdata[63:0]);
				end else if (axis_client_tvalid && axis_client_tready) begin
					item_count <= item_count - 1;
					//$display("Client: Sent %d to Adder!", axis_client_tdata[63:0]);
				end
				
				if (client_tlast) begin
					last_item_latch <= 1'b1;
				end
        end
    end
endmodule
