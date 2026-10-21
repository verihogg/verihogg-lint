module m;
  logic clk;
  always #5 clk = ~clk;
endmodule
