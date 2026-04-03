module test;
  int a;
  initial begin
    if (a inside { {1,2}, {3,4} }) begin end
  end
endmodule