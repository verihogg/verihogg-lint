module repetition_in_sequence_decl;
  logic clk, req, ack;

  sequence s_goto;
    req [->3] ##1 ack;
  endsequence

  sequence s_nonconsec;
    req [=2] ##1 ack;
  endsequence

  property p;
    @(posedge clk) s_goto;
  endproperty

  assert property (p);
endmodule
