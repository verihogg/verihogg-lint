function int some_func(int a);
  return a + 1;
endfunction

module top;
  int x;
  initial x = $unit::some_func(5);
endmodule