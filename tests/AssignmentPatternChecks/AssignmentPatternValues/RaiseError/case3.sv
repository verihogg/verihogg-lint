// Single-member struct, 2 values given
module top;
  struct packed {
    logic [7:0] val;
  } s = '{8'hAA, 8'hBB};
endmodule
