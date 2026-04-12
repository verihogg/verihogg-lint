// 4-member struct, 4 values
module top;
  struct packed {
    logic [7:0] a;
    logic [7:0] b;
    logic [7:0] c;
    logic [7:0] d;
  } s = '{8'h11, 8'h22, 8'h33, 8'h44};
endmodule
