module m;
  logic en;
  initial begin
    fork : w
      #10;
    join_none
    if (en) disable w;
  end
endmodule
