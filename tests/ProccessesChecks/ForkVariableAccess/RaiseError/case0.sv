module m;
  logic a, b;
  initial begin
    fork : w
      a = b;
    join : w
  end
endmodule
