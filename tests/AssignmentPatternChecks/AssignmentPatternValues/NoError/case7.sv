// Unpacked array - pattern fills the unpacked dimension, not the packed one
module top;
  logic [31:0] arr[2];
  assign arr = '{32'h0, 32'h1};
endmodule