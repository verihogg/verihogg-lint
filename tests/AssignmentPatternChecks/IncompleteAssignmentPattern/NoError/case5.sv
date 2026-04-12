// 4-member struct, all members provided
module top;
  struct packed {
    logic [7:0] a;
    logic [7:0] b;
    logic [7:0] c;
    logic [7:0] d;
  } s = '{a: 8'h11, b: 8'h22, c: 8'h33, d: 8'h44};
endmodule
