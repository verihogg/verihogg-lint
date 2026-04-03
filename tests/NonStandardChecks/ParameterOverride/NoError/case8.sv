module m #(parameter P1 = 1)();
endmodule

module top();
  m #(.P1(3)) u_m();
endmodule