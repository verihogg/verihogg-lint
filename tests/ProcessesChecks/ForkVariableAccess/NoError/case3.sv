module m;
  initial begin
    fork : w
      #10;
    join : w
  end
endmodule
