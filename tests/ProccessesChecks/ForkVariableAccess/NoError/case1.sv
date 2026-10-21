module m;
  const int c = 5;
  initial begin
    fork : w
      $display("%0d", c);
    join : w
  end
endmodule
