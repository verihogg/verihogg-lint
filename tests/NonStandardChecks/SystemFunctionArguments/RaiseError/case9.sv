module test_isunknown_fail;
  logic a, b;
  initial begin
    bit u = $isunknown(a, b);
  end
endmodule