checker Global();
  assert property (1);
endchecker

interface IF;
  checker Local();
    Global g();   
  endchecker
endinterface

module top;
  IF if1();
endmodule
