module test_high_fail;
  logic [7:0] arr [0:3];
  initial begin
    int h = $high(arr, 0, 1);
  end
endmodule