checker Inner();
  assert property (1);
endchecker

checker Outer();
  Inner i();  
endchecker

module top;
  Outer o();
endmodule
