module wildcard_eq_bad;
  initial begin
    logic [3:0] a = 4'b1010;
    logic [3:0] b = 4'b10x0;

    if (a =?= b)
      $display("match");
  end
endmodule
