module m;
  int x;

  function int value();
    return 1;
  endfunction

  initial begin
    x = $void(value());
  end
endmodule
