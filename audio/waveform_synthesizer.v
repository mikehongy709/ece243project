module waveform_synthesizer (resetn, CLOCK_50, en, sound_code, volume_code, sound);
  input CLOCK_50;
  input resetn;
  input en;
  input [3:0] sound_code;
  input [3:0] volume_code;
  output signed [31:0] sound;

  // array to hold the outputs of each waveform generator
  wire signed [31:0] sound_array [3:0];

  // loop to instantiate waveform generators
  genvar i;
  generate
    for (i = 0; i < 4; i = i + 1) begin : gen_waveform_readers
      waveform_reader #(.n(7 + i)) gen_instance (
        .resetn(resetn),
        .CLOCK_50(CLOCK_50),
        .enable(sound_code[i] & en),
        .sound(sound_array[i])
      );
    end
  endgenerate

  // scale the volume based on the volume code
  reg signed [31:0] scaled_sound_array [3:0];
  integer j;
  always @ (*) 
    for (j = 0; j < 4; j = j + 1) begin
      if (volume_code[j]) 
          scaled_sound_array[j] = sound_array[j]; // Keep original value
      else 
          scaled_sound_array[j] = 32'b0; // Scale down by 2
    end

  assign sound = scaled_sound_array[0] + scaled_sound_array[1] 
                + scaled_sound_array[2] + scaled_sound_array[3];

endmodule

module waveform_reader(resetn, CLOCK_50, enable, sound);
  parameter n = 8;
  input resetn, CLOCK_50, enable;
  output signed [31:0] sound;
  
  wire CLOCK_read; // clock that reads the data from memory

  CLOCK_Divider #(.n(n)) D1 (
    .resetn(resetn),
    .CLOCK_50(CLOCK_50),
    .enable(enable),
    .CLOCK_SLOW(CLOCK_read)
  );

  wire [7:0] addr_count;
  counter_nbit #(.n(8)) U2 (
    .CLOCK_50(CLOCK_read),
    .resetn(resetn),
    .en(1'b1),
    .count(addr_count)	
  );

  wire signed [31:0] sound_raw;
  ram256x32 U3 (
    .address(addr_count),
    .clock(CLOCK_read),
    .data(32'b0),
    .wren(1'b0),
    .q(sound_raw)
  );

  // divide the raw sound by 4 to reduce the volume 
  assign sound = sound_raw >>> 2;
endmodule

module CLOCK_Divider (resetn, CLOCK_50, enable, CLOCK_SLOW);
	parameter n = 8;
  input CLOCK_50, resetn, enable;
  output CLOCK_SLOW;

  reg [n-1:0] Count;

  always @ (posedge CLOCK_50) begin
    if (!resetn)
      Count <= {n{1'b0}};
    else if (enable)
      Count <= Count + 1;
  end

  assign CLOCK_SLOW = ~|Count;
endmodule

module counter_nbit (CLOCK_50, resetn, en, count);
  parameter n = 8;
	input CLOCK_50;       
	input resetn; // synchronous active low reset
  input en;
	output reg [n-1:0] count; // n-bit output for the counter value

	always @(posedge CLOCK_50)
		if (!resetn) 
			count <= 0; // Reset the counter to 0
		else if (en)
			count <= count + 1; // Increment the counter by 1
endmodule