module select_in_event_bad;
  logic [7:0] bus;
  logic       out;

  always @(bus[0])
    out = bus[0];
endmodule
