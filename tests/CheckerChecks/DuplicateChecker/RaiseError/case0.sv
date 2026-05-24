checker MyCheck(input bit a);
  assert property (a);
endchecker

checker MyCheck(input bit a, b);
  assert property (a == b);
endchecker

module top;
  MyCheck c1(1);
endmodule
