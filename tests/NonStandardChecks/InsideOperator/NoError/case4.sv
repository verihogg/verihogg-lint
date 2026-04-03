module test2;
  int a = 5;
  int b = 3;
  int c = 7;
  logic res;
  assign res = (a inside {b, c});
endmodule