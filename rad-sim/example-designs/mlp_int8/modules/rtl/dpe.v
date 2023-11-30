/**
Dot Product Engine Module
Performs a dot product calculation for two vectors

Parameters:
LANES: The number of elements the DPE can handle at once (Max number of elements in a subset of the input vector)
DATAW: Bit width of data
IPREC: Precision of elements in inputs to DPE
MPREC: Precision after multiplication
OPREC: Precision of elements in outputs from DPE
ADDER_STAGES: Number of stages required for hierarchical adding

Inputs:
i_valid: Valid signal for all data
i_dataa: Vector data
i_datab: Vector data

Outputs:
o_valid: Valid signal for output result
o_result: Result
**/
module dpe # (
	parameter LANES = 64,
	parameter DATAW = 512,
	parameter IPREC = 8,
	parameter MPREC = 2 * IPREC,
	parameter OPREC = 32,
	parameter ADDER_STAGES = $clog2(LANES)
)(
	input              clk,
	input              rst,
	input              i_valid,
	input  [DATAW-1:0] i_dataa,
	input  [DATAW-1:0] i_datab,
	output             o_valid,
	output [OPREC-1:0] o_result
);

// Input registers
wire signed [IPREC-1:0] dataa [0:LANES-1];
wire signed [IPREC-1:0] datab [0:LANES-1];
reg signed [IPREC-1:0] r_dataa [0:LANES-1];
reg signed [IPREC-1:0] r_datab [0:LANES-1];
reg r_ivalid;

// Multiplication registers
reg signed [MPREC-1:0] r_mrslt [0:LANES-1];
reg r_mvalid;

// Adder tree registers
reg signed [OPREC-1:0] r_arslt_s1 [0:31];
reg signed [OPREC-1:0] r_arslt_s2 [0:15];
reg signed [OPREC-1:0] r_arslt_s3 [0:7];
reg signed [OPREC-1:0] r_arslt_s4 [0:3];
reg signed [OPREC-1:0] r_arslt_s5 [0:1];
reg signed [OPREC-1:0] r_arslt_s6;
reg r_avalid [0:ADDER_STAGES-1];

genvar j;
generate
for (j = 0; j < LANES; j = j + 1) begin: split_input
	assign dataa[j] = i_dataa[(j+1)*IPREC-1:j*IPREC];
	assign datab[j] = i_datab[(j+1)*IPREC-1:j*IPREC];
end
endgenerate

integer i;
always @ (posedge clk) begin
	if (rst) begin
		for (i = 0; i < LANES; i = i + 1) begin
			r_mrslt[i] <= 'd0;
			r_dataa[i] <= 'd0;
			r_datab[i] <= 'd0;
		end
		r_ivalid <= 1'b0;
		r_mvalid <= 1'b0;
		for (i = 0; i < 32; i = i + 1) r_arslt_s1[i] <= 'd0;
		for (i = 0; i < 16; i = i + 1) r_arslt_s2[i] <= 'd0;
		for (i = 0; i <  8; i = i + 1) r_arslt_s3[i] <= 'd0;
		for (i = 0; i <  4; i = i + 1) r_arslt_s4[i] <= 'd0;
		for (i = 0; i <  2; i = i + 1) r_arslt_s5[i] <= 'd0;
		r_arslt_s6 <= 'd0;
		for (i = 0; i < ADDER_STAGES; i = i + 1) begin
			r_avalid[i] <= 1'b0;
		end
	end else begin
		// Register inputs
		for (i = 0; i < LANES; i = i + 1) begin
			r_dataa[i] <= dataa[i];
			r_datab[i] <= datab[i];
		end
		r_ivalid <= i_valid;
		
		// Perform multiplication
		for (i = 0; i < LANES; i = i + 1) begin
			r_mrslt[i] <= r_dataa[i] * r_datab[i];
		end
		r_mvalid <= r_ivalid;
		
		// Register adder tree
		for (i = 0; i < 32; i = i + 1) r_arslt_s1[i] <= r_mrslt[i] + r_mrslt[32+i];
		for (i = 0; i < 16; i = i + 1) r_arslt_s2[i] <= r_arslt_s1[i] + r_arslt_s1[16+i];
		for (i = 0; i <  8; i = i + 1) r_arslt_s3[i] <= r_arslt_s2[i] + r_arslt_s2[ 8+i];
		for (i = 0; i <  4; i = i + 1) r_arslt_s4[i] <= r_arslt_s3[i] + r_arslt_s3[ 4+i];
		for (i = 0; i <  2; i = i + 1) r_arslt_s5[i] <= r_arslt_s4[i] + r_arslt_s4[ 2+i];
		r_arslt_s6 <= r_arslt_s5[0] + r_arslt_s5[1];
		r_avalid[0] <= r_mvalid;
		for (i = 1; i < ADDER_STAGES; i = i + 1) begin
			r_avalid[i] <= r_avalid[i-1];
		end
	end
end

assign o_result = r_arslt_s6;
assign o_valid  = r_avalid[ADDER_STAGES-1]; 

endmodule