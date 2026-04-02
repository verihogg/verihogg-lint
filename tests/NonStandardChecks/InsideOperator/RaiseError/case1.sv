module inside_operator_bad;
  parameter logic VALID = (4 inside {1, 2, 3, 4, 5});

  initial begin
    $display("VALID = %0b", VALID);
  end
endmodule
