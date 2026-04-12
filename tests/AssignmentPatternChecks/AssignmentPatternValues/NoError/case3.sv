// Named pattern — rule ignores it
module top;
  struct packed {
    logic [7:0] a;
    logic [7:0] b;
  } s = '{a: 8'hAA, b: 8'hBB};
endmodule
