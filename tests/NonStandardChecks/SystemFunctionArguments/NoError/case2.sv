module test_typename_ok;
  logic [7:0] a;
  initial begin
    string s = $typename(a);
  end
endmodule