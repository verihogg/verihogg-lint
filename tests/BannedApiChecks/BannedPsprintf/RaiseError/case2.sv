module m;
  int count;
  initial begin
    $display($psprintf("count = %0d", count));
  end
endmodule
