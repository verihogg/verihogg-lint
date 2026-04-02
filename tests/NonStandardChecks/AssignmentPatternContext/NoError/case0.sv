module assignment_pattern_ctx_valid;
  typedef struct {
    int x;
    int y;
  } Point;

  initial begin
    Point p;
    p = '{x: 1, y: 2};
    $display("%0d %0d", p.x, p.y);
  end
endmodule
