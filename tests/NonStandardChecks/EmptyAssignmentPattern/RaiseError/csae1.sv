module empty_assign_pattern_bad;
  initial begin
    int arr[];
    arr = '{};
    $display("done");
  end
endmodule
