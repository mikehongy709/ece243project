module mem_audio (resetn, CLOCK_50, volume_code, sound);
  input resetn;
  input CLOCK_50;
  input [3:0] volume_code;
  output signed [31:0] sound;

  wire CLOCK_SLOW;
  wire play_sound;
  wire [3:0] beat_number;
  wire [3:0] sound_code;

  CLOCK_half_sec U1 (
    .resetn(resetn),
    .CLOCK_50(CLOCK_50),
    .CLOCK_SLOW(CLOCK_SLOW)
  );

  counter_nbit #(.n(4)) U2 (
    .CLOCK_50(CLOCK_50),
    .resetn(resetn),
    .en(CLOCK_SLOW),
    .count(beat_number)
  );

  timeline U3 (
    .resetn(resetn),
    .CLOCK_50(CLOCK_50), 
    .wren(1'b0),
    .din(4'b0),
    .beat_number(beat_number),
    .dout(sound_code)
  );

  sound_duration U4 (
    .resetn(resetn), 
    .CLOCK_50(CLOCK_50), 
    .en(CLOCK_SLOW),
    .play_sound(play_sound));

  waveform_synthesizer U5 (
    .resetn(resetn), 
    .CLOCK_50(CLOCK_50), 
    .en(play_sound),
    .sound_code(sound_code),
    .volume_code(volume_code),
    .sound(sound)
  );
endmodule

module timeline (resetn, CLOCK_50, wren, din, beat_number, dout);
  input resetn;
  input CLOCK_50;
  input wren;
  input [3:0] din;
  input [3:0] beat_number;
  output reg [3:0] dout;

  // 64-bit flip flop representing 16 4-bit flip flips
  reg [63:0] ff;

  always @ (posedge CLOCK_50) begin
    if (!resetn)
      ff <= 64'b0001001001001000100110110111010101111000011010000101010001111000;
    else if (wren)
      ff[beat_number * 4 +: 4] <= din;
  end

  // output 4-bits
  always @ (posedge CLOCK_50) begin
    dout = ff[beat_number * 4 +: 4];
  end
endmodule

module CLOCK_half_sec (resetn, CLOCK_50, CLOCK_SLOW);
  input CLOCK_50, resetn;
  output CLOCK_SLOW;

  reg [24:0] Count; 

  always @ (posedge CLOCK_50) begin
    if (!resetn)
      Count <= 0;
    else
      Count <= Count + 1;
  end
  assign CLOCK_SLOW = ~|Count;
endmodule

module sound_duration (resetn, CLOCK_50, en, play_sound);
  input resetn;
  input CLOCK_50;
  input en;
  output reg play_sound;
  
  reg [21:0] counter; 

  always @(posedge CLOCK_50) begin
    if (!resetn) begin
      counter <= 22'b0;    // Reset the counter
      play_sound <= 1'b0; // Reset the output signal
    end 
    else begin
      if (en && counter == 3'b0) begin
        // Start the playing when enabled and counter is zero
        counter <= 22'h3FFFFF; 
        play_sound <= 1'b1;
      end 
      else if (counter > 0) begin
        counter <= counter - 1;
        if (counter == 22'b1)
          play_sound <= 1'b0; // Deassert the output at the end
      end
    end
  end
endmodule