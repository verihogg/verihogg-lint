module test11;
  parameter int a = 1;
  parameter int b = 2;
  parameter int c = 3;

  localparam int SIZE = (a inside {b, c}) ? 4 : 8;
  int arr[SIZE];
endmodule