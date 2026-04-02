module scalar_assign_pattern_bad;
  initial begin
    logic flag;

    flag = '{1'b1};
    $display("flag = %0b", flag);
  end
endmodule
