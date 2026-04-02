module tb2;
  int field;

  covergroup cg(input int foo);
    coverpoint foo {
      ignore_bins ignore = foo with (field > 2);
    }
  endgroup
endmodule