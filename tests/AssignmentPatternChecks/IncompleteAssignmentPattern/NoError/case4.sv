// Multiple structs in same module, all complete
module top;
  struct packed {
    logic [7:0] x;
    logic [7:0] y;
  } p = '{x: 8'h01, y: 8'h02};

  struct packed {
    logic a;
    logic b;
    logic c;
  } q = '{a: 1'b0, b: 1'b1, c: 1'b0};
endmodule
