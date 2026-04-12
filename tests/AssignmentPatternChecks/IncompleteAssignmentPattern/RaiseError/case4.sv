// Typedef struct, one member missing out of 2
typedef struct packed {
  logic [7:0] r;
  logic [7:0] g;
} color_t;

module top;
  color_t c = '{r: 8'hFF};
endmodule
