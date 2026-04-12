typedef struct packed {
  logic [7:0] x;
  logic [7:0] y;
} point_t;

module top;
  point_t p = '{x: 8'h01, y: 8'h02};
endmodule
