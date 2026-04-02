module concat_multiplier_valid;
  parameter int N = 4;
  logic [7:0] byte_val = 8'hAB;

  initial begin
    logic [31:0] wide;
    wide = {4{byte_val}};
    $display("%0h", wide);

    wide = {N{byte_val}};
    $display("%0h", wide);
  end
endmodule
