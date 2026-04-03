package pkg1;

class A;
  extern function void foo();
endclass

endpackage

package pkg2;

function void pkg1::A::foo();
endfunction

endpackage