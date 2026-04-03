package pkg1;

class A;
  rand int x;
  extern constraint c;
endclass

endpackage

package pkg2;

constraint pkg1::A::c { x > 0; }

endpackage