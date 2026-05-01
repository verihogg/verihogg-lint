module test;
  initial begin
    $fatal("oops", "Wrong type");
  end
endmodule