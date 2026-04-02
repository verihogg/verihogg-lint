module bad_foreach_example;

  int array [0:3][0:7]; // двумерный массив
  int val;

  initial begin
    val = 1;

    foreach (array[i, j]) begin
      array[i][j] = i * 10 + j;
    end

    foreach (array[val][i]) begin
      $display("array[%0d][%0d] = %0d", val, i, array[val][i]);
    end

  end

endmodule