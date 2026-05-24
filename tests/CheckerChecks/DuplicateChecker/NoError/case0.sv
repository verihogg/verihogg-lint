checker AlwaysOne();
  assert property (1);
endchecker

checker AlwaysZero();
  assert property (0);
endchecker

module top;
  AlwaysOne c1();
  AlwaysZero c2();
endmodule
