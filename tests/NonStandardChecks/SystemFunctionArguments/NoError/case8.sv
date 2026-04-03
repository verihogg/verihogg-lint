module test_isunknown_ok;
  logic a;
  initial begin
    bit b = $isunknown(a);
  end
endmodule