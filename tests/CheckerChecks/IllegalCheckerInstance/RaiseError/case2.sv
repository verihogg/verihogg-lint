checker Helper();
  assert property (1);
endchecker

checker Main(input bit clk);
  Helper h();        
  assert property (clk);
endchecker

module top;
  Main m(1'b0);
endmodule
