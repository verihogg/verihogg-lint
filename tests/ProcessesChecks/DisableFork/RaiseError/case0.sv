module m;
  initial begin
    fork
      #10;
    join_none
    disable fork;
  end
endmodule
