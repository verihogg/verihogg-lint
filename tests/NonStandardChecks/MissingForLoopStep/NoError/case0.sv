module for_loop_step_valid;
  initial begin
    for (int i = 0; i < 8; i++)
      $display("i = %0d", i);
  end
endmodule
