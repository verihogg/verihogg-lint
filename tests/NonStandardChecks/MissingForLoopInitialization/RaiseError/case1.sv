module for_loop_no_init;
  initial begin
    int i;
    for (; i < 8; i++)
      $display("i = %0d", i);
  end
endmodule
