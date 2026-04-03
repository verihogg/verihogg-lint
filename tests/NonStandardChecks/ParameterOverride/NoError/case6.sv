module m #(parameter P1 = 1, parameter P2 = 2)();
endmodule

module top();
  m #(3, 5) u_m();
endmodule