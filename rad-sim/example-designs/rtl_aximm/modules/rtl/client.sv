module client # (
    parameter DATA = 64'hDEADBEEF,
    parameter DST_MEM_ADDR = 3,
    parameter DST_NODE = 64'h2,
    parameter SRC_NODE = 64'h0,
    parameter AXI4_IDW = 8,
    parameter AXI4_ADDRW = 64,
    parameter AXI4_LENW = 8,
    parameter AXI4_SIZEW = 3,
    parameter AXI4_BURSTW = 2,
    parameter AXI4_USERW = 64,
    parameter AXI4_MAX_DATAW = 512,
    parameter AXI4_RESPW = 2
) (
	input clk,
    input rst,
    input start_sig,
    output logic start_rdy,
    output logic output_valid,
    output logic output_result,
    input output_rdy,
    // AXI-MM Address Write Channel
    output logic aximm_client_awvalid,
    input aximm_client_awready,
    output logic [AXI4_IDW-1:0] aximm_client_awid,
    output logic [AXI4_ADDRW-1:0] aximm_client_awaddr,
    output logic [AXI4_LENW-1:0] aximm_client_awlen,
    output logic [AXI4_SIZEW-1:0] aximm_client_awsize,
    output logic [AXI4_BURSTW-1:0] aximm_client_awburst,
    output logic [AXI4_USERW-1:0] aximm_client_awuser,
    // AXI-MM Data Write Channel
    output logic aximm_client_wvalid,
    input aximm_client_wready,
    output logic [AXI4_IDW-1:0] aximm_client_wid,
    output logic [AXI4_MAX_DATAW-1:0] aximm_client_wdata,
    output logic aximm_client_wlast,
    output logic [AXI4_USERW-1:0] aximm_client_wuser,
    // AXI-MM Write Response Channel
    input aximm_client_bvalid,
    output logic aximm_client_bready,
    input [AXI4_IDW-1:0] aximm_client_bid,
    input [AXI4_RESPW-1:0] aximm_client_bresp,
    input [AXI4_USERW-1:0] aximm_client_buser,
    // AXI-MM Address Read Channel
    output logic aximm_client_arvalid,
    input aximm_client_arready,
    output logic [AXI4_IDW-1:0] aximm_client_arid,
    output logic [AXI4_ADDRW-1:0] aximm_client_araddr,
    output logic [AXI4_LENW-1:0] aximm_client_arlen,
    output logic [AXI4_SIZEW-1:0] aximm_client_arsize,
    output logic [AXI4_BURSTW-1:0] aximm_client_arburst,
    output logic [AXI4_USERW-1:0] aximm_client_aruser,
    // AXI-MM Data Read Channel
    input aximm_client_rvalid,
    output logic aximm_client_rready,
    input [AXI4_IDW-1:0] aximm_client_rid,
    input [AXI4_MAX_DATAW-1:0] aximm_client_rdata,
    input [AXI4_RESPW-1:0] aximm_client_rresp,
    input aximm_client_rlast,
    input [AXI4_USERW-1:0] aximm_client_ruser
);
    localparam RST_STATE=3'b000, AW_STATE=3'b001, W_STATE=3'b010, B_STATE=3'b011, AR_STATE=3'b100, R_STATE=3'b101, END_STATE=3'b110;
    localparam DEST_ADDR = DST_NODE << (AXI4_ADDRW-4) | DST_MEM_ADDR;
    localparam SRC_ADDR = SRC_NODE << (AXI4_ADDRW-4);

    logic [2:0] current_state;
    logic [2:0] next_state;

    always_comb begin
        case (current_state)
            RST_STATE: next_state = AW_STATE;
            AW_STATE: next_state = W_STATE;
            W_STATE: next_state = B_STATE;
            B_STATE: next_state = AR_STATE;
            AR_STATE: next_state = R_STATE;
            R_STATE: next_state = END_STATE;
            default: next_state = END_STATE;
        endcase
    end

    always_ff @(posedge clk) begin
        if (rst) begin
            current_state <= RST_STATE;
            start_rdy <= 1'b0;
            output_result <= 1'b0;
            output_valid <= 1'b0;
            aximm_client_awvalid <= 1'b0;
            aximm_client_awid <= 0;
            aximm_client_awaddr <= 0;
            aximm_client_awlen <= 0;
            aximm_client_awsize <= 0;
            aximm_client_awburst <= 0;
            aximm_client_awuser <= 0;
            aximm_client_wvalid <= 1'b0;
            aximm_client_wid <= 0;
            aximm_client_wdata <= 0;
            aximm_client_wlast <= 1'b0;
            aximm_client_wuser <= 0;
            aximm_client_bready <= 1'b0;
            aximm_client_arvalid <= 1'b0;
            aximm_client_arid <= 0;
            aximm_client_araddr <= 0;
            aximm_client_arlen <= 0;
            aximm_client_arsize <= 0;
            aximm_client_arburst <= 0;
            aximm_client_aruser <= 0;
            aximm_client_rready <= 1'b0;
        end else begin
            start_rdy <= 1'b1;
			if (current_state == RST_STATE) begin
                output_result <= 1'b0;
                output_valid <= 1'b0;
                aximm_client_awvalid <= 1'b0;
                aximm_client_awid <= 0;
                aximm_client_awaddr <= 0;
                aximm_client_awlen <= 0;
                aximm_client_awsize <= 0;
                aximm_client_awburst <= 0;
                aximm_client_awuser <= 0;
                aximm_client_wvalid <= 1'b0;
                aximm_client_wid <= 0;
                aximm_client_wdata <= 0;
                aximm_client_wlast <= 1'b0;
                aximm_client_wuser <= 0;
                aximm_client_bready <= 1'b0;
                aximm_client_arvalid <= 1'b0;
                aximm_client_arid <= 0;
                aximm_client_araddr <= 0;
                aximm_client_arlen <= 0;
                aximm_client_arsize <= 0;
                aximm_client_arburst <= 0;
                aximm_client_aruser <= 0;
                aximm_client_rready <= 1'b0;
                if (start_sig) begin // wait for driver to start
                    current_state <= next_state;
                end
            end else if (current_state == AW_STATE) begin
                aximm_client_awaddr <= DEST_ADDR;
                aximm_client_awid <= 0;
                aximm_client_awlen <= 0;
                aximm_client_awburst <= 0;
                aximm_client_awsize <= 3'b110;
                aximm_client_awuser <= SRC_ADDR;

                if (aximm_client_awvalid && aximm_client_awready) begin
                    aximm_client_awvalid <= 1'b0;
                    current_state <= next_state;
                    $display("Sent AW Transaction!");
                end else begin
                    aximm_client_awvalid <= 1'b1;
                end
            end else if (current_state == W_STATE) begin
                aximm_client_wid <= 0;
                aximm_client_wdata <= DATA;
                aximm_client_wlast <= 1'b1;
                aximm_client_wuser <= SRC_ADDR;
                
                if (aximm_client_wvalid && aximm_client_wready) begin
                    aximm_client_wvalid <= 1'b0;
                    current_state <= next_state;
                    $display("Sent W Transaction: %h!", DATA);
                end else begin
                    aximm_client_wvalid <= 1'b1;
                end
            end else if (current_state == B_STATE) begin
                aximm_client_bready <= 1'b1;

                if (aximm_client_bvalid && aximm_client_bready) begin
                    aximm_client_bready <= 1'b0;

                    current_state <= next_state;
                    $display("Received B Transaction!");
                end
            end else if (current_state == AR_STATE) begin
                aximm_client_araddr <= DEST_ADDR;
                aximm_client_arid <= 0;
                aximm_client_arlen <= 0;
                aximm_client_arburst <= 0;
                aximm_client_arsize <= 3'b110;
                aximm_client_aruser <= SRC_ADDR;
                
                if (aximm_client_arvalid && aximm_client_arready) begin
                    aximm_client_arvalid <= 1'b0;
                    current_state <= next_state;
                    $display("Sent AR Transaction!");
                end else begin
                    aximm_client_arvalid <= 1'b1;
                end
            end else if (current_state == R_STATE) begin
                aximm_client_rready <= 1'b1;

                if (aximm_client_rvalid && aximm_client_rready) begin
                    aximm_client_rready <= 1'b0;

                    current_state <= next_state;
                    $display("Received R Transaction: %h!", aximm_client_rdata);

                    output_result <= aximm_client_rdata;
                    output_valid <= 1'b1;
                end
            end
        end
    end
endmodule
