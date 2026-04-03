module m #(parameter P1 = 1)();
endmodule

module top();
  m #(1 + 2) u_m();
endmodule