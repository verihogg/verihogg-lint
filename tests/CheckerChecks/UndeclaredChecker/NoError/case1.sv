checker MyChecker(input bit a, b);
  assert property (a == b);
endchecker

module top;
  MyChecker chk(1, 0);
endmodule
