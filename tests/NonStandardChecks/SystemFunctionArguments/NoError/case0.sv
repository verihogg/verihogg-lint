module sysf_args_valid;
  logic [7:0] data [4];

  initial begin
    int w, d;
    w = $bits(data);
    d = $size(data, 1);
    $display("bits=%0d size=%0d", w, d);
  end
endmodule
