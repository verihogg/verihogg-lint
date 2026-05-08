module sub(input logic a, input logic b);
endmodule

module top;
  logic a, b;
  sub dut(.*, .*);
endmodule
