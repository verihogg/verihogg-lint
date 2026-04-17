class Processor;

  function void reset();
  endfunction

  function void run();
    void'(reset());
  endfunction

endclass

module top;
  initial begin
    Processor p = new();
    p.run();
  end
endmodule