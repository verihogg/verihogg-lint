class my_class;
  static function int foo();
    return 1;
  endfunction
endclass

module top;
  int x;
  initial x = my_class::foo();
endmodule