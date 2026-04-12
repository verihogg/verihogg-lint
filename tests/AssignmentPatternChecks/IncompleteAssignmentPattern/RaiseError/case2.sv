// 2-member struct, only one member provided
module top;
  struct packed {
    logic [7:0] x;
    logic [7:0] y;
  } p = '{x: 8'hAB};
endmodule
