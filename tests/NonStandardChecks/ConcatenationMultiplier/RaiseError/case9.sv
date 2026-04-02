module test7;
  parameter int P = 4;
  int v = 2;
  logic [7:0] w = {P+v{1'b1}};
endmodule