module implicit_data_type_valid (
  input  wire       clk,
  input  wire       rst_n,
  output wire [7:0] dout
);
  wire        internal_net;
  var  logic  my_var;
  wire [3:0]  bus;

  assign internal_net = clk & rst_n;
  assign dout = {4'b0, bus};
endmodule
