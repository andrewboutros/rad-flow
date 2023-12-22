module server # (
    parameter MEM_SIZE = 64,
    parameter MEM_ADDRW = $clog2(MEM_SIZE),
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
    // AXI-MM Address Write Channel
    input aximm_server_awvalid,
    output logic aximm_server_awready,
    input [AXI4_IDW-1:0] aximm_server_awid,
    input [AXI4_ADDRW-1:0] aximm_server_awaddr,
    input [AXI4_LENW-1:0] aximm_server_awlen,
    input [AXI4_SIZEW-1:0] aximm_server_awsize,
    input [AXI4_BURSTW-1:0] aximm_server_awburst,
    input [AXI4_USERW-1:0] aximm_server_awuser,
    // AXI-MM Data Write Channel
    input aximm_server_wvalid,
    output logic aximm_server_wready,
    input [AXI4_IDW-1:0] aximm_server_wid,
    input [AXI4_MAX_DATAW-1:0] aximm_server_wdata,
    input aximm_server_wlast,
    input [AXI4_USERW-1:0] aximm_server_wuser,
    // AXI-MM Write Response Channel
    output logic aximm_server_bvalid,
    input aximm_server_bready,
    output logic [AXI4_IDW-1:0] aximm_server_bid,
    output logic [AXI4_RESPW-1:0] aximm_server_bresp,
    output logic [AXI4_USERW-1:0] aximm_server_buser,
    // AXI-MM Address Read Channel
    input aximm_server_arvalid,
    output logic aximm_server_arready,
    input [AXI4_IDW-1:0] aximm_server_arid,
    input [AXI4_ADDRW-1:0] aximm_server_araddr,
    input [AXI4_LENW-1:0] aximm_server_arlen,
    input [AXI4_SIZEW-1:0] aximm_server_arsize,
    input [AXI4_BURSTW-1:0] aximm_server_arburst,
    input [AXI4_USERW-1:0] aximm_server_aruser,
    // AXI-MM Data Read Channel
    output logic aximm_server_rvalid,
    input aximm_server_rready,
    output logic [AXI4_IDW-1:0] aximm_server_rid,
    output logic [AXI4_MAX_DATAW-1:0] aximm_server_rdata,
    output logic [AXI4_RESPW-1:0] aximm_server_rresp,
    output logic aximm_server_rlast,
    output logic [AXI4_USERW-1:0] aximm_server_ruser
);
    localparam RST_STATE = 4'b0000,
               AW_STATE = 4'b0001,
               W_STATE = 4'b0010,
               B_STATE = 4'b0011,
               AR_STATE = 4'b0100,
               MEM_SETTLE1_STATE = 4'b0101,
               MEM_SETTLE2_STATE = 4'b0110,
               R_STATE = 4'b0111,
               END_STATE = 4'b1000;

    logic [3:0] current_state;
    logic [3:0] next_state;

    logic [MEM_ADDRW-1:0] memory_waddr;
    logic memory_wen;
    logic [AXI4_MAX_DATAW-1:0] memory_wdata;
    logic [MEM_ADDRW-1:0] memory_raddr;
    logic [AXI4_MAX_DATAW-1:0] memory_rdata;
    logic [MEM_ADDRW-1:0] r_memory_raddr;
    logic [MEM_ADDRW-1:0] rr_memory_raddr;

    logic [AXI4_USERW-1:0] dest_addr;

    memory #(
        .DATAW(AXI4_MAX_DATAW),
        .DEPTH(MEM_SIZE)
    ) aximm_memory (
        .clk(clk),
	    .rst(rst),
	    .waddr(memory_waddr),
	    .wen(memory_wen),
	    .wdata(memory_wdata),
	    .raddr(memory_raddr),
	    .rdata(memory_rdata)
    );

    always_comb begin
        case (current_state)
            RST_STATE: next_state = AW_STATE;
            AW_STATE: next_state = W_STATE;
            W_STATE: next_state = B_STATE;
            B_STATE: next_state = AR_STATE;
            AR_STATE: next_state = MEM_SETTLE1_STATE;
            MEM_SETTLE1_STATE: next_state = MEM_SETTLE2_STATE;
            MEM_SETTLE2_STATE: next_state = R_STATE;
            R_STATE: next_state = END_STATE;
            default: next_state = END_STATE;
        endcase
    end

    always_ff @(posedge clk) begin
        if (rst) begin
            aximm_server_awready <= 1'b0;
            aximm_server_wready <= 1'b0;
            aximm_server_bvalid <= 1'b0;
            aximm_server_bid <= 0;
            aximm_server_bresp <= 0;
            aximm_server_buser <= 0;
            aximm_server_arready <= 1'b0;
            aximm_server_rvalid <= 1'b0;
            aximm_server_rid <= 0;
            aximm_server_rdata <= 0;
            aximm_server_rresp <= 0;
            aximm_server_rlast <= 1'b0;
            aximm_server_ruser <= 0;

            dest_addr <= 0;

            current_state <= RST_STATE;
        end else begin
            if (current_state == RST_STATE) begin
                aximm_server_awready <= 1'b0;
                aximm_server_wready <= 1'b0;
                aximm_server_bvalid <= 1'b0;
                aximm_server_bid <= 0;
                aximm_server_bresp <= 0;
                aximm_server_buser <= 0;
                aximm_server_arready <= 1'b0;
                aximm_server_rvalid <= 1'b0;
                aximm_server_rid <= 0;
                aximm_server_rdata <= 0;
                aximm_server_rresp <= 0;
                aximm_server_rlast <= 1'b0;
                aximm_server_ruser <= 0;

                dest_addr <= 0;

                current_state <= next_state;
            end else if (current_state == AW_STATE) begin
                aximm_server_awready <= 1'b1;
                if (aximm_server_awready && aximm_server_awvalid) begin
                    aximm_server_awready <= 1'b0;
                    memory_waddr <= aximm_server_awaddr;
                    current_state <= next_state;
                end
            end else if (current_state == W_STATE) begin
                aximm_server_wready <= 1'b1;
                if (aximm_server_wready && aximm_server_wvalid) begin
                    aximm_server_wready <= 1'b0;
                    memory_wdata <= aximm_server_wdata;
                    memory_wen <= 1'b1;
                    dest_addr <= aximm_server_wuser;
                    current_state <= next_state;
                end
            end else if (current_state == B_STATE) begin
                memory_wen <= 1'b0;
                aximm_server_bid <= 0;
                aximm_server_buser <= dest_addr;
                aximm_server_bresp <= 0;

                if (aximm_server_bvalid && aximm_server_bready) begin
                    aximm_server_bvalid <= 1'b0;
                    current_state <= next_state;
                end else begin
                    aximm_server_bvalid <= 1'b1;
                end
            end else if (current_state == AR_STATE) begin
                aximm_server_arready <= 1'b1;
                if (aximm_server_arready && aximm_server_arvalid) begin
                    aximm_server_arready <= 1'b0;
                    memory_raddr <= aximm_server_araddr[MEM_ADDRW-1:0];
                    dest_addr <= aximm_server_aruser;
                    current_state <= next_state;
                end
            end else if (current_state == MEM_SETTLE1_STATE || current_state == MEM_SETTLE2_STATE) begin
                current_state <= next_state;
            end else if (current_state == R_STATE) begin
                aximm_server_rid <= 0;
                aximm_server_rdata <= memory_rdata;
                aximm_server_rlast <= 1'b1;
                aximm_server_ruser <= dest_addr;
                aximm_server_rresp <= 0;

                if (aximm_server_rvalid && aximm_server_rready) begin
                    aximm_server_rvalid <= 1'b0;
                    current_state <= next_state;
                end else begin
                    aximm_server_rvalid <= 1'b1;
                end
            end
        end
    end
endmodule
