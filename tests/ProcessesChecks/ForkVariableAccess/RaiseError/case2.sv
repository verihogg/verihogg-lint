module m;
  int x;
  initial begin
    fork : w
      x <= 42;
    join : w
  end
endmodule
