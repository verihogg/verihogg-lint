module m #(parameter P1 = 1)();
endmodule

module top();
  m # 3.14 u_m();
endmodule