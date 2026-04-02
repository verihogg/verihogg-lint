module for_loop_valid;
  initial begin
    for (int i = 0; i < 8; i++)
      $display("i = %0d", i);

    for (int j = 10; j > 0; j -= 2)
      $display("j = %0d", j);
  end
endmodule
