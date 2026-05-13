// 2-element unpacked array, but 3 values provided
module top;
  logic [31:0] arr[2];
  assign arr = '{32'h0, 32'h1, 32'h2};
endmodule