class Logger;

  function void log(string msg);
    $display("[LOG] %s", msg);
  endfunction

  function void log_error(string msg);
    void'(log({"ERROR: ", msg}));
  endfunction

  function void log_warning(string msg);
    void'(log({"WARN: ", msg}));
  endfunction

endclass

module top;
  initial begin
    Logger l = new();
    l.log_error("something went wrong");
  end
endmodule