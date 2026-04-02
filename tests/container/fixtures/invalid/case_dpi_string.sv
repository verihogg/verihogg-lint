module dpi_bad_string;
  import "DPI-C++" function int c_add(input int a, b);

  initial begin
    int result = c_add(3, 4);
    $display("result = %0d", result);
  end
endmodule
