module top;

  function void print_message(string msg);
    $display("MSG: %s", msg);
  endfunction

  initial begin
    void'(print_message("hello"));
  end

endmodule