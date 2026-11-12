module m;
  function bit do_rand();
    return 1;
  endfunction

  initial begin
    $void(do_rand());
  end
endmodule
