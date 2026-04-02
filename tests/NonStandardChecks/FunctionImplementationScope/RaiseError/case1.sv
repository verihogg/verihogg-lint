package my_pkg;

  class Adder;
    int a, b;

    extern function int get_sum();
  endclass

endpackage

import my_pkg::*; 

function int Adder::get_sum();

  return a + b;
endfunction

module top;
  my_pkg::Adder ad = new();
  initial begin
    ad.a = 3; ad.b = 5;
    $display("%0d", ad.get_sum());
  end
endmodule