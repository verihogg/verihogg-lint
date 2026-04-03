module child(input int x[2], input int y[2]);
endmodule

module top;
  int a[2], b[2];

  child u1(.x({a,b}), .y('{1,2}));
endmodule