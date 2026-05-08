module top;

  function void print_message(string msg);
    $display("MSG: %s", msg);
  endfunction

  initial begin
    print_message("hello");
  end

endmodule
