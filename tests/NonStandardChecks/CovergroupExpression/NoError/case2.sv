module tb1;
  int field;

  covergroup cg(input int foo);
    coverpoint foo {
      ignore_bins ignore = foo with (foo > 2);
    }
  endgroup
endmodule