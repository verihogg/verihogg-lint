module inner #(parameter int W = 8);
  logic [W-1:0] data;
endmodule

module param_override_valid;
  inner #(.W(16)) u_inner();
endmodule
