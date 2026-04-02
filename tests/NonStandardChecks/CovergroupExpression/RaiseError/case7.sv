module tb6;
  int field;

  covergroup cg(input int foo);
    coverpoint foo {
      bins b1 = foo with (foo == 3 && field > 0);
    }
  endgroup
endmodule