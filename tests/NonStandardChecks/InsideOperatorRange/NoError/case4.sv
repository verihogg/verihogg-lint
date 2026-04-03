module test;
  int a;
  initial begin
    if (a inside {[1:5], 10}) begin end
  end
endmodule