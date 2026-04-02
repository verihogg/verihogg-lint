module type_casting_valid;
  initial begin
    real   r = 3.75;
    int    i;
    logic  l;

    i = int'(r);
    l = logic'(1'b1);
    $display("i=%0d", i);
  end
endmodule
