module test;
  typedef struct { int x; int y; int z; } vec3_t;

  function vec3_t zero();
    return '{0, 0, 0};
  endfunction

  function vec3_t from_xy(int x, int y);
    return '{x, y, 0};
  endfunction
endmodule
