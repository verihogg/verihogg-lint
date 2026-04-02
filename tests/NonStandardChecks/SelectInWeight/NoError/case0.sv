module select_in_weight_valid;
  initial begin
    int x;
    randcase
      10: x = 1;
      20: x = 2;
      70: x = 3;
    endcase
    $display("x = %0d", x);
  end
endmodule
