module fatal_valid;
  initial begin
    assert (1 == 1) else $fatal(0, "level 0 fatal");
    assert (1 == 1) else $fatal(1, "level 1 fatal");
    assert (1 == 1) else $fatal(2, "level 2 fatal");
  end
endmodule
