module tb5;
  covergroup cg(input int foo, input int bar);
    coverpoint foo {
      bins b1 = foo with (foo == 3);
    }
    coverpoint bar {
      bins b2 = bar with (bar < 5);
    }
  endgroup
endmodule