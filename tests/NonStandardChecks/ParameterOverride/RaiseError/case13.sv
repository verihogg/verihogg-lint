`define VAL 5

module m #(parameter P1 = 1)();
endmodule

module top();
  m # `VAL u_m();
endmodule