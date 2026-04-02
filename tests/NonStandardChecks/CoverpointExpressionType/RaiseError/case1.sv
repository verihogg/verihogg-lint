module coverpoint_bad;
  real voltage;

  covergroup cg;
    cp_volt: coverpoint voltage;
  endgroup

  cg cg_inst = new();
endmodule
