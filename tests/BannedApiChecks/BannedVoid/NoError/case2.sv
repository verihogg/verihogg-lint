module m;
  int t;

  function void do_nothing();
  endfunction

  initial begin
    ;
    do_nothing();
    t = $time;
    t = $random;
  end
endmodule
