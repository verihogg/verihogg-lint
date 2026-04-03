module test3;
  int a, b, c;
  logic res;

  always_comb begin
    res = (a inside {b, c});
  end
endmodule