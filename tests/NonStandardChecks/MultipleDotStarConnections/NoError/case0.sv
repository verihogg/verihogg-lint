module child (
  input  logic clk,
  input  logic rst_n,
  output logic [7:0] dout
);
  assign dout = 8'hAB;
endmodule

module dot_star_valid;
  logic       clk, rst_n;
  logic [7:0] dout;

  // Single .* — allowed
  child u_child (.*, .dout(dout));
endmodule
