module m;
  initial begin
    fork : w
      #10;
    join_any
    disable w;
  end
endmodule
