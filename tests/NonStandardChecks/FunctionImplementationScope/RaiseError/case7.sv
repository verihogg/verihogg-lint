package pkg;

class A;
  extern function void foo();
endclass

endpackage

module m;

function void pkg::A::foo();
endfunction

endmodule