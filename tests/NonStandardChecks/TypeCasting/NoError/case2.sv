module correct_cast_1;
  int a;
  int b;
  initial begin
    b = int'(a);
  end
endmodule