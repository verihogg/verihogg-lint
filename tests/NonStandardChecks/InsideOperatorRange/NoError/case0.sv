module inside_operator_range_valid;
  initial begin
    int x = 5;
    if (x inside {[1:10]})
      $display("x is in range");

    if (x inside {1, 2, 3, 4, 5})
      $display("exact match");
  end
endmodule
