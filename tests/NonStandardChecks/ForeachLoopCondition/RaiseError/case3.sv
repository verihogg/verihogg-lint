module test_foreach_2d_wrong;
  int arr[3][4];

  initial begin
    foreach (arr[i][j]) begin
      $display("arr[%0d][%0d] = %0d", i, j, arr[i][j]);
    end
  end
endmodule