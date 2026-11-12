class program_like;
  function void run_program();
  endfunction
endclass

module top;
  initial begin
    program_like p = new();
    p.run_program();
  end
endmodule
