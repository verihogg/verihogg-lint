module empty_assign_pattern_valid;
  initial begin
    int arr[3];
    arr = '{1, 2, 3};
    $display("%0d %0d %0d", arr[0], arr[1], arr[2]);
  end
endmodule
