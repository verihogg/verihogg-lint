module test;
  typedef struct { int a; int b; } s_t;
  s_t s;

  always_ff @(posedge clk) begin
    s <= '{1, 2};
  end
endmodule