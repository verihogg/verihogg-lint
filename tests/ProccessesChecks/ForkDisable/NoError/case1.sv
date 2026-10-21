module m;
  initial begin
    fork : w
      begin
        #10;
        disable w;
      end
    join
  end
endmodule
