module m;
  logic clk, q, d;
  always_ff @(posedge clk) q <= d;
endmodule
