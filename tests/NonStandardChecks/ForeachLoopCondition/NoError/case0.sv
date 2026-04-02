module foreach_valid;
  int mat[2][3];
  initial begin
    int arr[4];
    arr = '{10, 20, 30, 40};

    foreach (arr[i])
      $display("arr[%0d] = %0d", i, arr[i]);

    mat = '{ {1,2,3}, {4,5,6} };

    foreach (mat[i,j])
      $display("mat[%0d][%0d] = %0d", i, j, mat[i][j]);
  end
endmodule