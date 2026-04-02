module param_fixed_array #(
  parameter int WIDTH = 8,
  parameter logic [3:0] INIT_VAL [4] = '{4'h0, 4'h1, 4'h2, 4'h3}
);
  logic [WIDTH-1:0] data;
endmodule
