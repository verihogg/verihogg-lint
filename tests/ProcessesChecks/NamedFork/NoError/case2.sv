module m;
  initial begin
    fork : w
      #10;
    join_none
  end
endmodule
