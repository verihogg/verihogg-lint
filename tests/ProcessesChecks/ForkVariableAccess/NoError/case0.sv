module m;
  parameter int P = 5;
  localparam int Q = 10;
  initial begin
    fork : w
      $display("%0d %0d", P, Q);
    join : w
  end
endmodule
