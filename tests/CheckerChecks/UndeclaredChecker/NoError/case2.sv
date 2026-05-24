package pkg;
  checker PkgCheck();
    assert property (1);
  endchecker
endpackage

module top;
  import pkg::*;
  PkgCheck c();
endmodule
