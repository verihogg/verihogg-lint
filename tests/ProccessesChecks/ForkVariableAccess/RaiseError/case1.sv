module m;
  int x;
  initial begin
    fork : w
      $display("%0d", x);
    join : w
  end
endmodule
