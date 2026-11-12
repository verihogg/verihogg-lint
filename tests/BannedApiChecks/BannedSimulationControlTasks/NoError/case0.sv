module my_module;
  initial begin
    $display("$finish");
    $fatal(1, "error");
  end
endmodule
