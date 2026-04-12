// Positional pattern — rule ignores it
module top;
  struct packed {
    logic [7:0] a;
    logic [7:0] b;
  } s = '{8'hAA, 8'hBB};
endmodule
