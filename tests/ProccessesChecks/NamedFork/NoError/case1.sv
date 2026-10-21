module m;
  initial begin
    fork : w
      #5;
    join_any : w
  end
endmodule
