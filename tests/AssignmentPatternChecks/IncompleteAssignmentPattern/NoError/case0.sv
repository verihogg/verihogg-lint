module top;
  struct packed {
    logic [7:0] a;
    logic [7:0] b;
    logic [7:0] c;
  } s = '{a: 8'hAA, b: 8'hBB, c: 8'hCC};
endmodule
