package pkg;
  checker Common();
    assert property (1);
  endchecker
endpackage

checker Common();
  assert property (0);
endchecker

module top;
  pkg::Common c1();
  Common c2();
endmodule
