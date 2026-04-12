// Multiple variables, each with correct count
module top;
  struct packed {
    logic [7:0] x;
    logic [7:0] y;
  } p = '{8'h01, 8'h02};

  struct packed {
    logic a;
    logic b;
    logic c;
  } q = '{1'b0, 1'b1, 1'b0};
endmodule
