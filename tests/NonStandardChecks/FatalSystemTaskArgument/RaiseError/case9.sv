module test;
  int lvl = 2;
  initial begin
    $fatal(lvl + 1, "Computed level");
  end
endmodule