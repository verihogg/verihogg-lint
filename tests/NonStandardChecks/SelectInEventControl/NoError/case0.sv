module select_in_event_valid;
  logic clk;
  logic [7:0] bus;
  logic out;

  always @(posedge clk)
    out <= bus[0];

  always @(bus)
    out <= |bus;
endmodule
