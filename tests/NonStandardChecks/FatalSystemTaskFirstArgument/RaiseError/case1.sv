module fatal_bad_arg;
  initial begin
    assert (1 == 1) else $fatal(5, "invalid severity level");
  end
endmodule
