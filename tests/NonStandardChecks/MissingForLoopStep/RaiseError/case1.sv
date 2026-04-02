module for_loop_no_step;
  initial begin
    for (int i = 0; i < 8;) begin
      $display("i = %0d", i);
      i++;
    end
  end
endmodule
