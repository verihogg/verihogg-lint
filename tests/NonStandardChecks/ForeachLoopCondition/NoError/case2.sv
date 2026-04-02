module test_foreach_1d_correct;
  int arr[5];

  initial begin
    foreach (arr[i]) begin
      arr[i] = i;
    end
  end
endmodule