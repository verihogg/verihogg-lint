module UsedModule();
endmodule

checker TopChecker();
  UsedModule u();   // модуль объявлен
  assert property (1);
endchecker

module top;
  TopChecker tc();
endmodule
