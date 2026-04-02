module test3;
  localparam int M = 2+2;
  logic [7:0] x = {M{1'b1}};
endmodule