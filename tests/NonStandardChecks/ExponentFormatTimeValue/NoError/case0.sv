`timescale 1ns/1ps
module exponent_time_valid;
  initial begin
    #10ns;
    #2.5ns;
    #100ps;
    $finish;
  end
endmodule
