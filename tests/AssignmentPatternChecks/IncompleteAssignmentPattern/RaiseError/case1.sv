typedef struct packed {
  logic [7:0] x;
  logic [7:0] y;
  logic [7:0] z;
} vec3_t;

module top;
  vec3_t v = '{x: 8'h01};
endmodule
