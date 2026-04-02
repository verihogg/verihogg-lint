module test2;
  parameter int N = 8;
  logic [7:0] x = {N{1'b0}}; // параметр — константное выражение
endmodule