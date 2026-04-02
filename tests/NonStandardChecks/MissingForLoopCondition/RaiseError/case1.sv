module for_loop_no_cond;
  initial begin
    for (int i = 0;; i++) begin
      $display("i = %0d", i);
      if (i >= 7) break;
    end
  end
endmodule
