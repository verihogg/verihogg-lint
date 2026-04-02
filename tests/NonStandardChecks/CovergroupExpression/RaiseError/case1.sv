module cg_expr_fail_with_expr;

  int field;

  covergroup cg with function sample(int foo);
    cp: coverpoint foo {
      ignore_bins ign = foo with (field > 2);
    }
  endgroup

endmodule