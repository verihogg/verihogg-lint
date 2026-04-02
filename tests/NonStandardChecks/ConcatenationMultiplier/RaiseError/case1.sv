module concat_multiplier_bad;
  logic [7:0] byte_val = 8'hAB;

  initial begin
    int          n = 4;
    logic [31:0] wide;
    wide = {n{byte_val}};
    $display("%0h", wide);
  end
endmodule
