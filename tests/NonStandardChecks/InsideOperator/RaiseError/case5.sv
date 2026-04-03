module test5;
  parameter int a = 1;
  parameter int b = 2;
  parameter int c = 3;

  parameter int P = (a inside {b, c}) ? 1 : 0;
endmodule