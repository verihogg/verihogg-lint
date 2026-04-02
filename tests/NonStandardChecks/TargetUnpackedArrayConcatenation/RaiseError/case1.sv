module target_unpacked_bad;
  initial begin
    logic [7:0] arr[3];
    logic [7:0] other[3];

    {arr, other} = '{8'h01, 8'h02, 8'h03, 8'h04, 8'h05, 8'h06};
  end
endmodule
