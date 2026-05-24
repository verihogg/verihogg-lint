checker A();
  assert property (1);
endchecker

checker B();
  A a();   
endchecker

checker C();
  B b();   
endchecker

module top;
  C c();
endmodule
