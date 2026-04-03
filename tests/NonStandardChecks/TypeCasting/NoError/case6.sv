module correct_cast_3;
  int a, b;
  initial begin
    b = int'(a + 1);
  end
endmodule