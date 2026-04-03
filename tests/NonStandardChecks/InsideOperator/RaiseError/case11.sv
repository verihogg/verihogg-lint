module test9;
  parameter int a = 1;
  parameter int b = 2;
  parameter int c = 3;

  localparam int X = (a inside {b, c}) ? 5 : 10;
endmodule