// default: key — rule ignores non-member keys
module top;
  struct packed {
    logic [7:0] a;
    logic [7:0] b;
    logic [7:0] c;
  } s = '{default: 8'h00};
endmodule
