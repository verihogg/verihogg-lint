interface MyIF;
  checker IFCheck();
    assert property (1);
  endchecker
  IFCheck c();
endinterface

module top;
  MyIF if1();
endmodule
