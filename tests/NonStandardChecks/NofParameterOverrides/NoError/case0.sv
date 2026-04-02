module inner #(
  parameter int W = 8,
  parameter int D = 4
);
  logic [W-1:0] mem [D];
endmodule

module nof_param_overrides_valid;
  inner #(16, 8) u_inner();
endmodule
