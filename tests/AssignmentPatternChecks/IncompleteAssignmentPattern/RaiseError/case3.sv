// 4-member struct, 2 members missing
module top;
  struct packed {
    logic [7:0] a;
    logic [7:0] b;
    logic [7:0] c;
    logic [7:0] d;
  } s = '{a: 8'h11, c: 8'h33};
endmodule
