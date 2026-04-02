module implicit_data_type_bad (
  input  wire clk,
  output [7:0] dout 
);
  [3:0] internal_bus;

  assign dout = {4'b0, internal_bus};
endmodule
