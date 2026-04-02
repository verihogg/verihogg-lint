module assignment_pattern_ctx_bad;
  typedef struct {
    int x;
    int y;
  } Point;

  function void print_point(Point p);
    $display("%0d %0d", p.x, p.y);
  endfunction

  initial begin
    print_point('{1, 2});
  end
endmodule
