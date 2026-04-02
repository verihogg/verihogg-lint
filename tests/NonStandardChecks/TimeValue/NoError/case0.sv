`timescale 1ns/1ps
module time_value_valid;
  initial begin
    #10ns;
    #5.5ns;
    #100ps;
    $finish;
  end
endmodule
