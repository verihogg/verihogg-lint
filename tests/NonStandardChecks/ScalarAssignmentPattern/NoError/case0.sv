module scalar_assign_pattern_valid;
  initial begin
    logic [3:0] nibble;
    logic [7:0] byte_val;

    nibble    = '{1'b0, 1'b0, 1'b0, 1'b0};
    byte_val  = '{1'b1, 1'b1, 1'b1, 1'b1, 1'b1, 1'b1, 1'b1, 1'b1};
    $display("%0h %0h", nibble, byte_val);
  end
endmodule
