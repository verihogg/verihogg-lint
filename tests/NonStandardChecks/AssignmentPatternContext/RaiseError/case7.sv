module test;
  function void foo(int a, int b);
  endfunction

  initial begin
    foo('{1, 2});
  end
endmodule