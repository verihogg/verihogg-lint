module m;
  initial begin
    fork
      #5;
    join_any
  end
endmodule
