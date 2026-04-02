module repetition_bad_example;
  logic clk, req, ack;

  property p_req_ack;
    @(posedge clk) req [->3] |-> ##1 ack;
  endproperty

  assert property (p_req_ack);
endmodule