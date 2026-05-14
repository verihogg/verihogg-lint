module repetition_valid_seq_decl;
  logic clk, req, ack;

  sequence s_consec;
    req [*3] ##1 ack;
  endsequence

  property p;
    @(posedge clk) s_consec;
  endproperty

  assert property (p);
endmodule
