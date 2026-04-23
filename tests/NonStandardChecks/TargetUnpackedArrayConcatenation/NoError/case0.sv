module target_unpacked_valid;
  initial begin
    logic [7:0] arr[3];
    logic [7:0] a, b, c;

    arr[0] = 8'h01;
    arr[1] = 8'h02;
    arr[2] = 8'h03;

    {a, b, c} = {arr[0], arr[1], arr[2]};

    $display("%0h %0h %0h", a, b, c);
  end
endmodule
