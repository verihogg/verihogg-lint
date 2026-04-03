module test_size_ok;
  logic [7:0] arr [0:3];
  initial begin
    int sz = $size(arr, 0);
  end
endmodule