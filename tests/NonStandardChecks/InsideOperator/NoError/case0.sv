module inside_operator_valid;
  initial begin
    int x = 5;

    if (x inside {1, 2, 3, 4, 5})
      $display("x is in set");

    assert (x inside {[0:10]});
  end
endmodule
