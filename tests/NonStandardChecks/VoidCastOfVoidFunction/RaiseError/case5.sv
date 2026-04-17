class Monitor;

  function void check_protocol(logic [7:0] data);
    if (data === 8'hFF)
      $error("Protocol violation");
  endfunction

  function void sample(logic [7:0] data);
    void'(check_protocol(data));
  endfunction

endclass

module top;
  initial begin
    Monitor m = new();
    m.sample(8'hFF);
  end
endmodule