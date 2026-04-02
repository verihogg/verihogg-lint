module scalar_assign_pattern_valid;
  initial begin
    logic [3:0] nibble;
    logic [7:0] byte_val;

    nibble    = '{4'h0};
    byte_val  = '{8'hFF};
    $display("%0h %0h", nibble, byte_val);
  end
endmodule
