module test6;
  localparam int a = 1;
  localparam int b = 2;
  localparam int c = 3;

  localparam int P = (a inside {b, c}) ? 1 : 0;
endmodule