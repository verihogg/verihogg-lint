module m #(parameter P1 = 1)();
endmodule

module top();
  m # 3 u_m();
endmodule