module tb4;
  int global_var;

  covergroup cg(input int foo);
    coverpoint foo {
      bins b1 = foo with (global_var < 5);
    }
  endgroup
endmodule