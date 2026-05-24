interface MyIF;
  checker MyChecker();
    assert property (1);
  endchecker
  MyChecker c();  
endinterface

module top;
  MyIF if1();
endmodule
