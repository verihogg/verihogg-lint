// Typedef struct: expected 2, got 3
typedef struct packed {
  logic [7:0] r;
  logic [7:0] g;
} color_t;

module top;
  color_t c = '{8'hFF, 8'h00, 8'h80};
endmodule
