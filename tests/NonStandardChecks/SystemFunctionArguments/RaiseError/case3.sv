module test_typename_fail;
  logic [7:0] a, b;
  initial begin
    string s = $typename(a, b);
  end
endmodule